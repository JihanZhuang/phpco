#ifndef _LINK_H_
#define _LINK_H_
#include<stdio.h>  
#include<stdlib.h>  
#include<malloc.h>  
typedef char datatype;
typedef struct NODE  
{  
    datatype data;  
    struct NODE *next;  
}node;  
typedef struct  
{  
    node *front;  
    node *rear;  
    int size;  
}Link;
Link *initqueue();
int emptyqueue(Link *p);
int getlength(Link *p);
void pushqueue(Link *p, datatype data);
void delete_node(Link *p,datatype data);
#endif
