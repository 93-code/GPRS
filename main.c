#include <head.h>
#include "queue.h"

pthread_mutex_t  send_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t  wake_lock = PTHREAD_MUTEX_INITIALIZER;

pthread_cond_t   send_cond = PTHREAD_COND_INITIALIZER;
pthread_cond_t   wake_cond = PTHREAD_COND_INITIALIZER;

enum{CMD1='1',CMD2='2',QUIT='3'};

int Index;
char *msg[] = {"cmd1","cmd2","quit"};

void *send_thread(void *arg)
{
	int fd;
	char *filename = (char  *)arg;

	fd = open(filename,O_WRONLY | O_CREAT | O_TRUNC,0666);
	if(fd < 0){
		fprintf(stderr,"Fail to open %s : %s\n",filename,strerror(errno));
		exit(EXIT_FAILURE);
	}

	while(1)
	{
		pthread_cond_wait(&send_cond,&send_lock);
		write(fd,msg[Index],strlen(msg[Index]));
		pthread_mutex_unlock(&send_lock);

		if(Index == 2)
			break;
	}

	pthread_exit(NULL);
}


void *wake_thread(void *arg)
{
	LinkQueue *q = (LinkQueue *)arg;

	while(1)
	{
		pthread_mutex_lock(&wake_lock);

		if(is_empty_linkqueue(q))
			pthread_cond_wait(&wake_cond,&wake_lock);
		
		switch(delete_linkqueue(q))
		{
		case  CMD1:
			Index = 0;
			break;
		
		case  CMD2:
			Index = 1;
			break;

		case  QUIT:
			Index = 2;
			break;
		}

		pthread_mutex_unlock(&wake_lock);

		pthread_mutex_lock(&send_lock);
		//唤醒发送线程 
		pthread_cond_signal(&send_cond);
		pthread_mutex_unlock(&send_lock);
		
		usleep(500);

		if(Index == 2)
			break;
	}

	pthread_exit(NULL);
}

//./a.out  log.txt fifo
int main(int argc, const char *argv[])
{
	int n;
	int fd;
	int ret;
	char cmd;
	pthread_t tid[2];
	LinkQueue *q = create_linkqueue();

	if(argc < 3)
	{
		fprintf(stderr,"Usage : %s log.txt fifo\n",argv[0]);
		exit(EXIT_FAILURE);
	}

	ret = pthread_create(&tid[0],NULL,send_thread,(void *)argv[1]);
	if(ret != 0){
		fprintf(stderr,"Fail to pthread_create : %s\n",strerror(ret));
		exit(EXIT_FAILURE);
	}

	ret = pthread_create(&tid[1],NULL,wake_thread,q);
	if(ret != 0){
		fprintf(stderr,"Fail to pthread_create : %s\n",strerror(ret));
		exit(EXIT_FAILURE);
	}

	if(mkfifo(argv[2],0666)  < 0 && errno != EEXIST)
	{
		fprintf(stderr,"Fail to mkfifo : %s\n",strerror(errno));
		exit(EXIT_FAILURE);
	}

	fd = open(argv[2],O_RDONLY);
	if(fd < 0){
		handler_error(argv[2]);
	}

	while(1)
	{
		n = read(fd,&cmd,1);
		if(n == 0)
			break;

		//添加到任务队列 
		enter_linkqueue(q,cmd);
	
		pthread_mutex_lock(&wake_lock);
		//唤醒wake_thread
		pthread_cond_signal(&wake_cond);
		pthread_mutex_unlock(&wake_lock);
		
		usleep(500);

		if(cmd == QUIT)
			break;
	}
	
	pthread_join(tid[0],NULL);
	pthread_join(tid[1],NULL);
	
	return 0;
}
