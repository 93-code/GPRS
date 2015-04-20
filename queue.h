#ifndef _HEAD_H_ 
#define _HEAD_H_ 


typedef char DATATYPE;

//链表节点的类型
typedef struct node 
{
	DATATYPE data;
	struct node *next;
}LinkNode;

///////////////////////////////////////////////////////////////////
typedef struct 
{
	LinkNode *front;
	LinkNode *rear;
}LinkQueue;

extern LinkQueue *create_linkqueue();
extern int is_empty_linkqueue(LinkQueue *q);
extern int enter_linkqueue(LinkQueue *q,DATATYPE data);
extern DATATYPE delete_linkqueue(LinkQueue *q);
#endif 
