#include <time.h>
#include <unistd.h>
#include "headers.h"
#include "Queue.h"
#define null 0

key_t msgqid;
struct processData //define process data
{
    int arrivaltime;
    int priority;
    int runningtime;
    int id;
    int memsize;

};

struct msgbuff
{
    long mtype;
    struct processData mData;
};

void clearResources(int); //function to clear ipc and clock recources

int main(int argc, char * argv[])
{
    signal(SIGINT, clearResources);
    //Initialization
    int choice;
    FILE * pFile; //define a pointer to the file
    struct processData pData;
    int send_val;
    struct msgbuff message;
    msgqid = msgget(1000, IPC_CREAT | 0644); //create a message queue
    //define the input queue
    Queue Input_Queue;
    queueInit(&Input_Queue,sizeof(struct processData));

    // 1. Read the input files.
    pFile = fopen("processes.txt", "r");
    while (!feof(pFile))
    {
        fscanf(pFile,"%d\t%d\t%d\t%d\t%d\n", &pData.id, &pData.arrivaltime, &pData.runningtime, &pData.priority,&pData.memsize);
        printf("MEMory Size %d \n", pData.memsize);

        enqueue(&Input_Queue,&pData);
        
    }

    // 2. Ask the user for the chosen scheduling algorithm and its parameters.
    printf("Please enter the scheduling algorithm you want 1:SJF 2:SRTN :\n");
    scanf("%d", &choice);
    printf("\n the choice is: %d\n",choice);
    char pCount[100];
    sprintf(pCount,"%d",getQueueSize(&Input_Queue));
    int pid1 = fork(); //fork the scheduler process
    printf("\nPID1 is: %d\n",pid1);
    switch(choice){
        case 1:
        //SJF
        if (pid1 == -1)
            perror("error in fork");

        else if (pid1 == 0)
        {
            char *argv[] = { "./scheduler.out","1", pCount, 0 };
            execve(argv[0], &argv[0], NULL); //run the scheduler
        }
        break;

        case 2:
        if (pid1 == -1)
            perror("error in fork");

        else if (pid1 == 0)
        {
            printf("\n test case 2 \n");
            char *argv[] = { "./scheduler.out", "2",pCount, 0 };
            execve(argv[0], &argv[0], NULL);
        }
        break;
    }
    
    // 3. Initiate and create the scheduler and clock processes.
   
    int pid2 = fork(); //fork the clock process
    if (pid2 == -1)
        perror("error in fork");
       
    else if (pid2 == 0)
    {
        char *argv[] = { "./clk.out", 0 };
        execve(argv[0], &argv[0], NULL); //run the clock
    }

    //4. Initializing clock
    initClk();
    
    if(msgqid == -1)
    {
        perror("Error in create");
        exit(-1);
    }
    while(getQueueSize(&Input_Queue))
    {
        int x = getClk();
        queuePeek(&Input_Queue,&pData); //peek the first entry in the input queue
        //send the process at the appropriate time.
        if (pData.arrivaltime==x)
        {
        dequeue(&Input_Queue,&pData);
        printf("current time is %d\n", x);
        struct msgbuff message;
        message.mtype = 7;     	/* arbitrary value */
        //message.mData = pData;
        message.mData.arrivaltime=pData.arrivaltime;
        message.mData.id=pData.id;
        message.mData.priority=pData.priority;
        message.mData.runningtime=pData.runningtime;
        message.mData.memsize = pData.memsize;
        
        send_val = msgsnd(msgqid, &message, sizeof(message.mData), !IPC_NOWAIT);
        
        //printf("\n%d %d %d %d\n",pData.id,pData.arrivaltime,pData.runningtime,pData.priority);
        printf("\nprocess with id = %d sent\n", pData.id);
        if(send_val == -1)
            perror("Errror in send");
        
        queuePeek(&Input_Queue,&pData);
        }
        else
        {
            sleep(1);
        }
    }
    // 7. Wait for the scheduler to exit
    int status;
    int pid = wait(&status);
    if(!(status & 0x00FF))
        printf("\nA scheduler with pid %d terminated with exit code %d\n", pid, status>>8);

    destroyClk(true);
}

void clearResources(int signum)
{
    //Clears all resources in case of interruption
    msgctl(msgqid, IPC_RMID, (struct msqid_ds *) 0); 
    exit(0);
}
