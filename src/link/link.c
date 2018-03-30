#include "php_coroutine.h"
Link *initqueue()      //构造一个空队列  
{  
    Link *p;  
    p = (Link *)malloc(sizeof(Link));  
    if(p == NULL)  
        exit(0);  //log
    p->front = NULL;
    p->rear = NULL;
    p->size = 0;  
    return p;  
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
void pushqueue(Link *p, datatype data)          // 将一个新元素入队  
{  
    node *que = (node *)malloc(sizeof(node));  
    que->data = data;  
    que->next = NULL;  
    if(emptyqueue(p)){  
        p->front = p->rear = que;  
    }
    else{  
        p->rear = que;  
    }  
    p->size++;  
}

void delete_node(Link *p,datatype data){
    node *q,*f=NULL;
    if(p->size==0){
        return;
    }
    q=p->front;
    while(q){
        if(q->data==data){
            if(q==p->front){
                p->front=p->front->next;
            }else if(q==p->rear){
                p->rear=f;
            }else{
                f->next=q->next;
            }
            free(q);
            p->size--;
            break;
        }else{
            f=q;
            q=q->next;
        }
    }
}

void printqueue(Link *p){
    node *q;
    q=p->front;
    while(q){
        //print(q)
        q=q->next;
    }
}  
  
