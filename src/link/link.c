#include<stdio.h>  
#include<stdlib.h>  
#include<malloc.h>  
typedef struct NODE  
{  
    int data;  
    struct NODE *next;  
}node;  
typedef struct  
{  
    struct NODE *front;  
    struct NODE *rear;  
    int size;  
}Link;  
Link *initqueue()      //构造一个空队列  
{  
    Link *p;  
    p = (Link *)malloc(sizeof(Link));  
    if(p == NULL)  
        exit(0);  
    p->front = NULL;  
    p->rear = NULL;  
    p->size = 0;  
    return p;  
}  
void destroyqueue(Link *p)     //销毁队列  
{  
    while(p->front)  
    {  
        p->rear = p->front->next;  
        free(p->front);  
        p->front = p->rear;  
    }  
}  
void clearqueue(Link *p)       //清空一个队列  
{  
    p->front = p->rear;  
    p->front->next = NULL;  
    p->size = 0;  
}  
int emptyqueue(Link *p)       //判断队列是否为空  
{  
    if(p->front == NULL && p->rear == NULL)  
        return 1;  
    return 0;  
}  
int getlength(Link *p)           //返回对列的长度  
{  
    return p->size;  
}  
int getfront(Link *p)            //返回队首元素  
{  
    int a;  
    if(p->front->next == NULL)  
        return 0;  
    a = p->front->data;  
    return a;  
}  
void pushqueue(Link *p, int data)          // 将一个新元素入队  
{  
    node *que = (node *)malloc(sizeof(node));  
    que->data = data;  
    que->next = NULL;  
    if(emptyqueue(p))  
        p->front = p->rear = que;  
    else  
    {  
        p->rear->next = que;  
        p->rear = que;  
    }  
    p->size++;  
}  
void popqueue(Link *p)          // 将队首元素出队  
{  
    if(p->front->next == NULL)  
        return;  
    node *que;  
    que = p->front;  
    p->front = que->next;  
    if(p->rear == que)  
        p->rear = NULL;  
    p->size--;  
    free(que);  
}  
