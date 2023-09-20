
#include <stdio.h>
#include <stdlib.h>
//Process data of Priority Queue
struct processData
{
    int arrivaltime;
    int priority;
    int runningtime;
    int id;
    int sizeP;
};
//Node data of Priority Queue
typedef struct nodePQ{
    int priority;
    struct processData data;
    struct nodePQ* next; 
} nodePQ;
//Priority Queue
typedef struct {
    nodePQ *head;
    int length;
} PQueue;
 
//Initialize the Priority Queue
void PQueueInit(PQueue *q)
{
   q->length = 0;
   q->head = NULL;
}

// Function to push according to priority 
void push(PQueue *h, int priority, struct processData data) 
{ 
    nodePQ* start = h->head; 
    nodePQ* temp = (nodePQ*)malloc(sizeof(nodePQ)); 
    temp->data = data; 
    temp->priority = priority; 
    temp->next = NULL; 
    
    if(h->head == NULL){
        h->head = temp;
        h->length++;
    }
    // Special Case: The head of list has lesser 
    // priority than new node. So insert new 
    // node before head node and change head node. 
    else if (h->head->priority > priority) { 
  
        // Insert New Node before head 
        temp->next = h->head; 
        h->head = temp;
        h->length++; 
    } 
    else { 
  
        // Traverse the list and find a 
        // position to insert new node 
        while (start->next != NULL && 
               start->next->priority < priority) { 
            start = start->next; 
        } 
  
        // Either at the ends of the list 
        // or at required position 
        temp->next = start->next; 
        start->next = temp; 
        h->length++;
    } 
}  


// Removes the element with the 
// highest priority form the list 
struct processData pop(PQueue *h) 
{ 
    if (!h->length) {
     struct processData NULL_processData = {-1,-1,-1,-1}; //the null value
        return NULL_processData  ; 
    }

    struct processData data = h->head->data;
    nodePQ* temp = h->head; 
    h->head = h->head->next; 
    h->length--;
    return data;
} 

int getlength(PQueue*h)
{
    return h->length;
}

int peak_time (PQueue *h){
   if(h->length != 0) {
       return h->head->data.arrivaltime;
    }
    else{
       return -1; 
    }   
}