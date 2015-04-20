#include <head.h>

int write_fifo(int fd)
{
	int n;
	char cmd;
	char buf[1024];

	while(1)
	{
		printf("Input cmd : ");

		cmd = getchar();
		getchar();

		write(fd,&cmd,1);

		if(cmd == '3')
			break;
	}

	return 0;
}

//./a.out  fifoname
int main(int argc, const char *argv[])
{
	int fd;

	if(mkfifo(argv[1],0666) < 0 && errno != EEXIST)
	{
		fprintf(stderr,"Fail to mkfifo %s : %s\n",argv[1],strerror(errno));
		return -1;
	}

	fd = open(argv[1],O_WRONLY);
	if(fd < 0){
		handler_error(argv[1]);
	}

	printf("fd = %d\n",fd);
	write_fifo(fd);

	return 0;
}
