#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

enum State{NotStarted, Waiting, Running, Finished};  //Status of processes
//Process data
struct PCB
{
    enum State state;
    int executionTime;
    int remainingTime;
    int waitingTime;
    int pid;
    int mem_address;

  
};
//Nodes of the LinedList
struct node {
   int id;
   struct  PCB data;
   struct node *next;
};

struct node *head = NULL;
struct node *current = NULL;

//Check if the list is empty
bool isEmpty() {
   return head == NULL;
}


//Insert node at the begining of the list
void insertFirst(int id, struct PCB data) {
   //Make a node with the its data
   struct node *link = (struct node*) malloc(sizeof(struct node));
   link->id = id;
   link->data = data;
   //Insert it to the begining of the list
   link->next = head;
   head = link;
}

//Delete first node of the list and return the data of that node
struct node* deleteFirst() {
   //save the data of deleted node to return it after deleteing 
   struct node *tempLink = head;
   //Advance the head pointer and return the deleted node
   head = head->next;
   return tempLink;
}

//Delete node with certain ID
struct node* delete(int id) {
   struct node* current = head;
   struct node* previous = NULL;
	
   //return null if the list is empty
   if(head == NULL) {
      return NULL;
   }
   //search for the desired node
   while(current->id != id) {
      //return null if the node is not found
      if(current->next == NULL) {
         return NULL;
      } else {
         previous = current;
         current = current->next;
      }
   }
   //Delete it and return the data of the node, if it is found
   if(current == head) {
      head = head->next;
   } else {
      previous->next = current->next;
   }    
	
   return current;
}

//Search for a node with certain ID
struct node* find(int id) {
   struct node* current = head;

   //return null if the list is empty
   if(head == NULL) {
      return NULL;
   }
   //search for the node
   while(current->id != id) {
      //return null if the node is not found
      if(current->next == NULL) {
         return NULL;
      } else {
         current = current->next;
      }
   }      
   //return the data of the node, if it is found
   return current;
}

