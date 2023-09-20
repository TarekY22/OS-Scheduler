#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//Node of the Queue
typedef struct Node
{
  void *data;
  struct Node *next;
}node;

//Queue List
typedef struct QueueList
{
    int sizeOfQueue;
    size_t size;
    node *head;
    node *tail;
}Queue;


//Initialize a new Queue
void queueInit(Queue *q, size_t size)
{
   q->sizeOfQueue = 0;
   q->size = size;
   q->head = q->tail = NULL;
}


//Return the data of peak of the queue
void queuePeek(Queue *q, void *data)
{
    if(q->sizeOfQueue > 0){
       node *temp = q->head;
       memcpy(data, temp->data, q->size);
    }
}


//Insert a new node at the end of the Queue
int enqueue(Queue *q, const void *data)
{
    node *newNode = (node *)malloc(sizeof(node));
    if(newNode == NULL){
        return -1;
    }
    newNode->data = malloc(q->size);
    if(newNode->data == NULL){
        free(newNode);
        return -1;
    }
    newNode->next = NULL;
    memcpy(newNode->data, data, q->size);
    if(q->sizeOfQueue == 0){
        q->head = q->tail = newNode;
    }
    else{
        q->tail->next = newNode;
        q->tail = newNode;
    }
    q->sizeOfQueue++;
    return 0;
}


//Remove a node from the begining of the Queue
void dequeue(Queue *q, void *data)
{
    if(q->sizeOfQueue > 0){
        node *temp = q->head;
        memcpy(data, temp->data, q->size);
        if(q->sizeOfQueue > 1){
            q->head = q->head->next;
        }
        else{
            q->head = NULL;
            q->tail = NULL;
        }
        q->sizeOfQueue--;
        free(temp->data);
        free(temp);
    }
}


//Remove nodes of the Queue
void clearQueue(Queue *q)
{
  node *temp;
  while(q->sizeOfQueue > 0){
      temp = q->head;
      q->head = temp->next;
      free(temp->data);
      free(temp);
      q->sizeOfQueue--;
      }
  q->head = q->tail = NULL;
}


//Length of the Queue
int getQueueSize(Queue *q)
{
    return q->sizeOfQueue;
}