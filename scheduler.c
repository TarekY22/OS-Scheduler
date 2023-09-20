#include "headers.h"
#include "Queue.h"
#include "LinkedList.h"
#include "PriorityQueue.h"

struct PCB * pcbPointer=NULL;
int t1 ,t2;
void Handler(int signum);
void Alarm_Handler(int alarmnum);
void ResumeProcess();

void SJF_Algorhithm(double *);
void SRTN_Algorithm(double *);
void HPF_Algorithm(double *);

void Merge(int address, int size);
void DeAllocate_memory(int address, int size);
int Allocate_memory(int size);


struct msgbuff
{
    long mtype;
    struct processData mData;
};

////////////Memory ////////////////////
//The values stored in the array refears to block's starting address in memory
int blocks_32[32];
int blocks_64[16];
int blocks_128[8];
int blocks_256[4] = {0, 256, 512, 768};
/////////////////////////////////////////


// Initialization 
 int rec_val,choice , pcount;
    double sum_WeightedTurnAround = 0 , sum_Waiting=0 , sum_RunningTime=0 ;
    key_t msgqid;
    struct msgbuff message;
    struct processData pData;
    struct PCB pcbBlock;
    struct node *pcbFind=NULL; // used down to get control block of the id 
    FILE * logFile;
    FILE * prefFile;
    FILE * sizeP;
    //SJF
    PQueue SJF_readyQueue;
    //SRTN
    PQueue SRTN_readyQueue;
    //
    PQueue HPF_readyQueue;
int main(int argc, char * argv[])
{
    initClk();
    signal(SIGUSR1,Handler); // when SIGUSER1 is raised when the process is terminated(in process.c) the handeler function is called 
    signal(SIGALRM, Alarm_Handler);


//Initialize for memory blocks all other arrays with -1
    for (int i = 0; i < 32; i++)
        blocks_32[i]=-1;
    for (int i = 0; i < 16; i++)
        blocks_64[i]=-1;
    for (int i = 0; i < 8; i++)
        blocks_128[i]=-1;

    //open the output files for write
    logFile = fopen("scheduler.log", "w");
    prefFile = fopen("scheduler.pref", "w");
    sizeP = fopen("memory.log", "w");
    
    fprintf(logFile, "\n#At time x process y state arr w total z remain y wait k\n");
    fprintf(sizeP, "\n#At time x allocated y bytes for process z from i to j\n"); 
        

    
    
    
    PQueueInit(&SJF_readyQueue);
    PQueueInit(&SRTN_readyQueue);
    PQueueInit(&HPF_readyQueue);

    
    sscanf(argv[2],"%d",&pcount);  // pcount is the number of processes (char *argv[] = { "./scheduler.out","1", pCount, 0 })
    int Num_processes=pcount;
    double WTAArray[pcount];
    //Receive the processes from message queue and add to ready queue.
    msgqid = msgget(1000, 0644);
    printf("\nthe chosen algorithm is: %s\n",argv[1]);
    sscanf(argv[1], "%d", &choice);
    switch(choice)
    {
        //SJF
        case 1:
            SJF_Algorhithm( WTAArray);
            break;
        //
        case 2:
        // Shortest Remaining time Next (SRTN)
            SRTN_Algorithm(WTAArray);
            break;
        case 3:
            HPF_Algorithm(WTAArray);
            break;
        

    }
    fclose(logFile);
    double stdWTA = 0.00;
    double avgWTA =(sum_WeightedTurnAround/Num_processes);
    for(int i = 0; i < Num_processes; i++){
        stdWTA += pow((WTAArray[i]-avgWTA),2);
    }
    fprintf(prefFile,"CPU Utilization = %.2f %%\n",(sum_RunningTime/getClk())*100);
    fprintf(prefFile,"Avg WTA = %.2f\n",(sum_WeightedTurnAround/Num_processes));
    fprintf(prefFile,"Avg Waiting = %.2f\n",sum_Waiting/Num_processes);
    fprintf(prefFile,"Std WTA = %.2f\n",sqrt(stdWTA/Num_processes));
    fclose(prefFile);
 
    //upon termination release the clock resources
    
    destroyClk(false);
}

void Handler(int signum)
{
    printf("Handler started\n");
    printf("from sig child pid is %d\n",pcbPointer->pid);
    int pid,stat_loc;
    pid = wait(&stat_loc);//stat_loc is the value of status of the termination of the process ; wait returns process id and status
    //If status (stat_loc) is returned normally from a terminated child process that returned a value of 0
    t2=getClk();
    if(WIFEXITED(stat_loc)){//WIFEXITED macro indicates if the child process exited normally or not (True or false) if yes we will use WEXITSTATUS
        if(WEXITSTATUS(stat_loc)==0) // the WEXITSTATUS macro returns the exit status of the child. Note it must be 0 if the child process is terminated normally
        {
            printf("\nProcess Finished\n");
            pcbPointer->state=Finished;
            pcbPointer->executionTime+=(t2-t1);
            pcbPointer->remainingTime=0;
        }
    }
}

void SJF_Algorhithm(double * WTAArray){
 ////////////////////////////////////////Reading data of each process in order to fork one with these specifications/////////////////////

        // wait for any process that's why IPCNOWAIT is off to suspend 
        rec_val= msgrcv(msgqid, &message, sizeof(message.mData), 0, !IPC_NOWAIT);
        //msgrcv function takes the message queue as input to read the processes parameters as arrival time etc., 
        //msgrcv(int msqid, void *msgp, size_t msgsz, long msgtyp, int msgflg)
        // msqid --> message queue that contains all the processes (id of the queue we put when we make it in process generator)
        // message buffer (msgp) which is a pointer that points to each process in the queue 
        // sizeof(message.mData) (size_t msgsz) --> size of the data 
        // long msgtyp it specifies whether you want to recive a message with a specific value or any message in the queue 
        // To recive any value msgtyp is zero , to recive a speceific type long msgtyp will be greater than 0 (the value of this type)
        // msgflg --> If the IPC_NOWAIT flag is off (!IPC_NOWAIT) in msgflg the calling process will suspend execution until one of the following occurs:
        //A message of the desired type is placed on the queue.
        //The message queue identifier, msgid, is removed from the system; when this occurs, errno is set to EIDRM and a value of -1 is returned.
        pData = message.mData;
        printf("\nTime = %d process with id = %d recieved\n", getClk(), pData.id);
        printf("MEM %d \n", pData.sizeP);

        //enqueue the process in the ready queue
        push(&SJF_readyQueue, pData.runningtime, pData); // prioirty here is the running time
       
        //Allocate Memory
        //Get the power of 2 value of size
        int size = pow(2,ceil(log(pData.sizeP)/log(2))); // if size = 20 --> log(20)/log(2) = 4.321 --> ceil(4.321) = 5 --> 2^5 = 32 
        int address = Allocate_memory(size);
        fprintf(sizeP, "\nAt time %d allocated %d bytes for process %d from %d to %d\n", getClk(), pData.sizeP, pData.id, address,address+size);
        
        //Create PCB
        pcbBlock.state = 0;
        pcbBlock.executionTime = 0;
        pcbBlock.remainingTime = pData.runningtime;
        pcbBlock.waitingTime = 0;
        pcbBlock.mem_address = address;

        insertFirst(pData.id, pcbBlock);
        printf("\nPCB created for process with id = %d\n", pData.id);
        while (pcount!=0)
        {
            printf("\nCurrent time = %d\n", getClk());
            rec_val = msgrcv(msgqid, &message, sizeof(message.mData), 0, IPC_NOWAIT);
            // If the IPC_NOWAIT flag is on in msgflg, the calling process will return immediately with a return value of -1 and errno set to ENOMSG. we do not suspend as we already waitung in while loop
            // if mesaage is recived 
            while (rec_val != -1)
            {
                pData = message.mData;
                printf("\nTime = %d process with id = %d recieved\n", getClk(), pData.id);
                push(&SJF_readyQueue, pData.runningtime, pData); //enqueue the data in the ready queue
                //Allocate Memory
                //Get the power of 2 value of size
                int size = pow(2,ceil(log(pData.sizeP)/log(2))); // if size = 20 --> log(20)/log(2) = 4.321 --> ceil(4.321) = 5 --> 2^5 = 32 
                int address = Allocate_memory(size);
                fprintf(sizeP, "\nAt time %d allocated %d bytes for process %d from %d to %d\n", getClk(), pData.sizeP, pData.id, address,address+size);
                //Creating PCB with the data we initial values in order to be edited down
                pcbBlock.state = 0;
                pcbBlock.executionTime = 0;
                pcbBlock.remainingTime = pData.runningtime;
                pcbBlock.waitingTime = 0;
                pcbBlock.mem_address = address;

            
                insertFirst(pData.id, pcbBlock);
                printf("\nPCB created for process with id = %d\n", pData.id);
                rec_val = msgrcv(msgqid, &message, sizeof(message.mData), 0, IPC_NOWAIT);
                
            }
            pData=pop(&SJF_readyQueue);
            //find the data of the deueued process
            pcbFind = find(pData.id);
            pcbPointer = &(pcbFind->data);
            pcbPointer->waitingTime=getClk()-(pData.arrivaltime)-(pcbPointer->executionTime); // change the intialized values of pcb waiting time variable
            // write in the output file the process data
            fprintf(logFile, "At time %d process %d started arr %d total %d remain %d wait %d\n", getClk(), pData.id, pData.arrivaltime, pData.runningtime, pcbPointer->remainingTime, pcbPointer->waitingTime);

            //forkprocess
            printf("\nIam forking a new process time = %d\n", getClk());
            int pid=fork();
            pcount--;
            pcbPointer->pid=pid;
            if (pid == -1)
            perror("error in fork");

            else if (pid == 0)
            {
                printf("\ntest the forking\n");
                char buf[20];
                sprintf(buf,"%d",pData.runningtime);
                char *argv[] = { "./process.out",buf, 0 }; // pass the running time to the process in order to be used in it 
                execve(argv[0], &argv[0], NULL); //execve() executes the program referred to by pathname. we run process.c
            }
            sleep(50);
            //Update pcb and calculate the process data after finishing it
            pcbPointer->executionTime=pData.runningtime;
            pcbPointer->waitingTime=getClk()-(pData.arrivaltime)-(pcbPointer->executionTime);
            pcbPointer->state=Finished;
            pcbPointer->remainingTime=0;
            int TA = getClk()-(pData.arrivaltime);
            double WTA=(double)TA/pData.runningtime;
            fprintf(logFile, "At time %d process %d finished arr %d total %d remain %d wait %d TA %d WTA %.2f\n", getClk(), pData.id, pData.arrivaltime, pData.runningtime, pcbPointer->remainingTime, pcbPointer->waitingTime, TA, WTA);
            sum_WeightedTurnAround+=WTA;
            WTAArray[pData.id] = WTA;
            sum_Waiting+=pcbPointer->waitingTime;
            sum_RunningTime+=pcbPointer->executionTime;
            //Deallocate the memory 
            size = pow(2,ceil(log(pData.sizeP)/log(2))); 
            DeAllocate_memory(pcbPointer->mem_address, size);
            fprintf(sizeP, "\nAt time %d deallocated %d bytes for process %d from %d to %d\n", getClk(), pData.sizeP, pData.id, pcbPointer->mem_address,pcbPointer->mem_address+size);
        }


    
}

void SRTN_Algorithm(double * WTAArray){
    //SRTN 
        //Recieving the processes sent by the process generator
        int size , address;
        rec_val = msgrcv(msgqid, &message, sizeof(message.mData), 0 , !IPC_NOWAIT);
        //msgrcv function takes the message queue as input to read the processes parameters as arrival time etc., 
        //msgrcv(int msqid, void *msgp, size_t msgsz, long msgtyp, int msgflg)
        // msqid --> message queue that contains all the processes (id of the queue we put when we make it in process generator)
        // message buffer (msgp) which is a pointer that points to each process in the queue 
        // sizeof(message.mData) (size_t msgsz) --> size of the data 
        // long msgtyp it specifies whether you want to recive a message with a specific value or any message in the queue 
        // To recive any value msgtyp is zero , to recive a speceific type long msgtyp will be greater than 0 (the value of this type)
        // msgflg --> If the IPC_NOWAIT flag is off (!IPC_NOWAIT) in msgflg the calling process will suspend execution until one of the following occurs:
        //A message of the desired type is placed on the queue.
        //The message queue identifier, msgid, is removed from the system; when this occurs, errno is set to EIDRM and a value of -1 is returned.
        printf("\nProcess with Pid = %d recieved\n",message.mData.id);
        pData = message.mData;
        push(&SRTN_readyQueue, pData.runningtime, pData); //enqueue the data in the ready queue
        if(rec_val!=-1)
        {

            //Allocate Memory
            //Get the power of 2 value of size
             size = pow(2,ceil(log(pData.sizeP)/log(2)));
             address = Allocate_memory(size);
            fprintf(sizeP, "\nAt time %d allocated %d bytes for process %d from %d to %d\n", getClk(), pData.sizeP, pData.id, address,address+size);


            //Creating PCB
            pcbBlock.state = 0;
            
            pcbBlock.executionTime = 0;
            
            pcbBlock.remainingTime = pData.runningtime;
            
            pcbBlock.waitingTime = 0;

            pcbBlock.mem_address=address;
            
            insertFirst(pData.id, pcbBlock);
            
            printf("\nPCB created for process with Pid = %d\n",pData.id);
        }

        while(getlength(&SRTN_readyQueue)!=0 || pcount!=0) // because pcount is decreased when the process has its first turn and not increased again so we must check the SRTN queue to know the remaining processes(That's why we check both of them)
        {
            rec_val = msgrcv(msgqid, &message, sizeof(message.mData), 0, IPC_NOWAIT);
            while(rec_val!=-1)
            {
                printf("\n Current time is: %d\n", getClk());
                printf("\nProcess with Pid = %d recieved\n",message.mData.id);
                pData = message.mData;
                push(&SRTN_readyQueue, pData.runningtime, pData); // push this new process to the queue to be forked down

                //Allocate Memory
                //Get the power of 2 value of size
                int size = pow(2,ceil(log(pData.sizeP)/log(2)));
                
                int address = Allocate_memory(size);
                
                fprintf(sizeP, "\nAt time %d allocated %d bytes for process %d from %d to %d\n", getClk(), pData.sizeP, pData.id, address,address+size);
            
                 //Creating PCB
                pcbBlock.state = 0;
                
                pcbBlock.executionTime = 0;
                
                pcbBlock.remainingTime = pData.runningtime;
                
                pcbBlock.waitingTime = 0;
                
                pcbBlock.mem_address = address;
                insertFirst(pData.id, pcbBlock);
                
                printf("\nPCB created for process with Pid = %d\n",pData.id);
                rec_val = msgrcv(msgqid, &message, sizeof(message.mData), 0, IPC_NOWAIT);
            }


            pData = pop (&SRTN_readyQueue);
            //The process took
            // first check if this process is finished or not (finished id = -1)
            if(pData.id == -1){  //if the process is finished skip this iteration 
                continue;
            }
            // if it is not finished go to PCB to get its data
            //find the PCB of the poped process
            pcbFind = find(pData.id);
            pcbPointer = &(pcbFind->data);
            //////////////////////////////
            // Now we will check if it is waiting 
            if(pcbPointer->state == Waiting)
            {
                fprintf(logFile, "At time %d process %d resumed arr %d total %d remain %d wait %d\n", getClk(), pData.id, pData.arrivaltime, pData.runningtime, pcbPointer->remainingTime, pcbPointer->waitingTime);
                printf("At time %d process %d resumed arr %d total %d remain %d wait %d\n", getClk(), pData.id, pData.arrivaltime, pData.runningtime, pcbPointer->remainingTime, pcbPointer->waitingTime);
                ResumeProcess();
                printf("\nCurrent time is: %d\n", getClk());
                alarm(1); 
                sleep(50);

                if(pcbPointer->state!=Finished)
                {
                    printf("\nThe process with Pid = %d is not finished yet \n",pcbPointer->pid);
                    push(&SRTN_readyQueue,pcbPointer->remainingTime , pData);
                    //Update PCB
                    pcbPointer->executionTime+=1;
                    
                    pcbPointer->waitingTime=getClk()-(pData.arrivaltime)-(pcbPointer->executionTime);
                    
                    pcbPointer->state=Waiting;
                    
                    pcbPointer->remainingTime-=1;
                   
                    fprintf(logFile, "At time %d process %d stopped arr %d total %d remain %d wait %d\n", getClk(), pData.id, pData.arrivaltime, pData.runningtime, pcbPointer->remainingTime, pcbPointer->waitingTime);
                    printf("At time %d process %d resumed arr %d total %d remain %d wait %d\n", getClk(), pData.id, pData.arrivaltime, pData.runningtime, pcbPointer->remainingTime, pcbPointer->waitingTime);
                }
                else
                {
                    pcbPointer->executionTime=pData.runningtime;

                    pcbPointer->waitingTime=getClk()-(pData.arrivaltime)-(pcbPointer->executionTime);
                    
                    pcbPointer->remainingTime=0;
                    
                    int TA = getClk()-(pData.arrivaltime);
                    
                    double WTA=(double)TA/pData.runningtime;
                    
                    fprintf(logFile, "At time %d process %d finished arr %d total %d remain %d wait %d TA %d WTA %.2f\n", 
                        getClk(), pData.id, pData.arrivaltime, pData.runningtime, pcbPointer->remainingTime, pcbPointer->waitingTime, TA,WTA);
                    printf("At time %d process %d finished arr %d total %d remain %d wait %d TA %d WTA %.2f\n", 
                        getClk(), pData.id, pData.arrivaltime, pData.runningtime, pcbPointer->remainingTime, pcbPointer->waitingTime, TA,WTA);
                    sum_WeightedTurnAround+=WTA;
                    WTAArray[pData.id] = WTA;

                    sum_Waiting+=pcbPointer->waitingTime;

                    sum_RunningTime+=pcbPointer->executionTime;
                    
                     //Deallocate memory for the finished process
                    size = pow(2,ceil(log(pData.sizeP)/log(2)));
                    DeAllocate_memory(pcbPointer->mem_address, size);
                    fprintf(sizeP, "\nAt time %d deallocated %d bytes for process %d from %d to %d\n", getClk(), pData.sizeP, pData.id, pcbPointer->mem_address,pcbPointer->mem_address+size);

                    pData.id=-1;
                }
            }    
            else // not waiting means that this is a new process that must be forked
            {
                //Fork the process
                printf("\nIam forking a new process \n");
                int pid=fork();
                pcbPointer->pid=pid;
                if (pid == -1)
                perror("error in forking");

                else if (pid == 0)
                {
                    printf("\ntesting the fork\n");
                    char buf[100];
                    sprintf(buf,"%d",pData.runningtime);
                    char *argv[] = { "./process.out",buf, 0 };
                    execve(argv[0], &argv[0], NULL);
                }

                pcount--;
                pcbPointer->waitingTime=getClk()-(pData.arrivaltime);
                printf("\nAt time %d process %d started arr %d total %d remain %d wait %d\n", getClk(), pData.id, pData.arrivaltime, pData.runningtime, pcbPointer->remainingTime, pcbPointer->waitingTime);
                fprintf(logFile, "At time %d process %d started arr %d total %d remain %d wait %d\n", getClk(), pData.id, pData.arrivaltime, pData.runningtime, pcbPointer->remainingTime, pcbPointer->waitingTime);
                alarm(1);
                sleep(50);  

                if(pcbPointer->state!=Finished)
                {
                    printf("Not Finished\n");

                    push (&SRTN_readyQueue, pcbPointer->remainingTime, pData);

                    pcbPointer->executionTime+=1;

                    pcbPointer->waitingTime=getClk()-(pData.arrivaltime)-(pcbPointer->executionTime);

                    pcbPointer->state=Waiting;

                    pcbPointer->remainingTime-=1;

                    fprintf(logFile, "At time %d process %d stopped arr %d total %d remain %d wait %d\n", getClk(), pData.id, pData.arrivaltime, pData.runningtime, pcbPointer->remainingTime, pcbPointer->waitingTime);
                    printf("At time %d process %d stopped arr %d total %d remain %d wait %d\n", getClk(), pData.id, pData.arrivaltime, pData.runningtime, pcbPointer->remainingTime, pcbPointer->waitingTime);
                }
                else
                {
                    pcbPointer->executionTime=pData.runningtime;

                    pcbPointer->waitingTime=getClk()-(pData.arrivaltime)-(pcbPointer->executionTime);

                    pcbPointer->remainingTime=0;

                    int TA = getClk()-(pData.arrivaltime);

                    double WTA=(double)TA/pData.runningtime;

                    fprintf(logFile, "At time %d process %d finished arr %d total %d remain %d wait %d TA %d WTA %.2f\n", 
                        getClk(), pData.id, pData.arrivaltime, pData.runningtime, pcbPointer->remainingTime, pcbPointer->waitingTime, TA, WTA);
                    printf("At time %d process %d finished arr %d total %d remain %d wait %d TA %d WTA %.2f\n", 
                        getClk(), pData.id, pData.arrivaltime, pData.runningtime, pcbPointer->remainingTime, pcbPointer->waitingTime, TA,WTA);
                    sum_WeightedTurnAround+=WTA;
                    WTAArray[pData.id] = WTA;

                    sum_Waiting+=pcbPointer->waitingTime;

                    sum_RunningTime+=pcbPointer->executionTime;

                    //Deallocate memory for the finished process
                    size = pow(2,ceil(log(pData.sizeP)/log(2)));
                    DeAllocate_memory(pcbPointer->mem_address, size);
                    fprintf(sizeP, "\nAt time %d deallocated %d bytes for process %d from %d to %d\n", getClk(), pData.sizeP, pData.id, pcbPointer->mem_address,pcbPointer->mem_address+size);

                    // to be checked above
                    pData.id=-1;
                         
            }   
        } 

}
}

void HPF_Algorithm(double * WTAArray)
{
     //Receive the process from the process generator queue
        rec_val= msgrcv(msgqid, &message, sizeof(message.mData), 0, !IPC_NOWAIT);
        if(rec_val!=0){
            printf("\nProcess with Pid = %d recieved\n",message.mData.id);
            pData = message.mData;

            //enqueue the process in the ready queue
            push(&HPF_readyQueue, pData.priority, pData);
            
            //Allocate Memory
            //Get the power of 2 value of size
            int size = pow(2,ceil(log(pData.sizeP)/log(2)));
            int address = Allocate_memory(size);
            fprintf(sizeP, "\nAt time %d allocated %d bytes for process %d from %d to %d\n", getClk(), pData.sizeP, pData.id, address,address+size);

            //Create PCB
            pcbBlock.state = 0;
            pcbBlock.executionTime = 0;
            pcbBlock.remainingTime = pData.runningtime;
            pcbBlock.waitingTime = 0;
            pcbBlock.mem_address = address;
            insertFirst(pData.id, pcbBlock);
            printf("\nPCB created for process with Pid = %d\n",pData.id);
        }
        
        while(getlength(&HPF_readyQueue)!=0 || pcount!=0)
        { //Receive a process
            rec_val = msgrcv(msgqid, &message, sizeof(message.mData), 0, IPC_NOWAIT);
            while(rec_val!=-1)
            {
                printf("\n Current time is: %d\n", getClk());
                printf("\nProcess with Pid = %d received \n",message.mData.id);
                pData = message.mData;
                // Enqueue in the ready queue
                push(&HPF_readyQueue, pData.priority,pData);
                
                //Allocate Memory
                //Get the power of 2 value of size
                int size = pow(2,ceil(log(pData.sizeP)/log(2)));
                int address = Allocate_memory(size);
                fprintf(sizeP, "\nAt time %d allocated %d bytes for process %d from %d to %d\n", getClk(), pData.sizeP, pData.id, address,address+size);
             //Create PCB
                pcbBlock.state = 0;
                pcbBlock.executionTime = 0;
                pcbBlock.remainingTime = pData.runningtime;
                pcbBlock.waitingTime = 0;
                pcbBlock.mem_address = address;
                insertFirst(pData.id, pcbBlock);
               printf("\nPCB created for process with Pid = %d\n",pData.id);
                
                //receive a process again to check that we took all the processes
                //from the process generator
                rec_val = msgrcv(msgqid, &message, sizeof(message.mData), 0, IPC_NOWAIT);
            }
            //pop the process with the highest priority
            pData=pop(&HPF_readyQueue);
            if(pData.id==-1){  //if the process is finished skip this iteration 
                continue;
            }
            //find the pcb of the deueued process
            pcbFind = find(pData.id);
            pcbPointer = &(pcbFind->data);
            if(pcbPointer->state == Waiting)
            {
                pcbPointer->waitingTime=getClk()-(pData.arrivaltime)-(pcbPointer->executionTime);
                fprintf(logFile, "At time %d process %d resumed arr %d total %d remain %d wait %d\n", getClk(), pData.id, pData.arrivaltime, pData.runningtime, pcbPointer->remainingTime, pcbPointer->waitingTime);
                ResumeProcess();
                printf("\nCurrent time is: %d\n", getClk());
                alarm(1); 
                sleep(50);
                if(pcbPointer->state!=Finished)
                {
                    printf("\nThe process with Pid = %d not finished yet \n",pcbPointer->pid);
                    push(&HPF_readyQueue, pData.priority, pData);
                    //Update PCB
                    pcbPointer->executionTime+=1;
                    pcbPointer->waitingTime=getClk()-(pData.arrivaltime)-(pcbPointer->executionTime);
                    pcbPointer->state=Waiting;
                    pcbPointer->remainingTime-=1;
                   
                    fprintf(logFile, "At time %d process %d stopped arr %d total %d remain %d wait %d\n", getClk(), pData.id, pData.arrivaltime, pData.runningtime, pcbPointer->remainingTime, pcbPointer->waitingTime);
                     
                }
                else
                {
                    pcbPointer->executionTime=pData.runningtime;
                    pcbPointer->waitingTime=getClk()-(pData.arrivaltime)-(pcbPointer->executionTime);
                    pcbPointer->remainingTime=0;
                    int TA = getClk()-(pData.arrivaltime);
                    double WTA=(double)TA/pData.runningtime;
                    // fprintf(logFile, "At time %d process %d finished arr %d total %d remain %d wait %d TA %d WTA %.2f\n", 
                    //     getClk(), pData.id, pData.arrivaltime, pData.runningtime, pcbPointer->remainingTime, pcbPointer->waitingTime, TA,WTA);
                    // sum_WTA+=WTA;
                    // sum_Waiting+=pcbPointer->waitingTime;
                    // sum_RunningTime+=pcbPointer->executionTime;
                    fprintf(logFile, "At time %d process %d finished arr %d total %d remain %d wait %d TA %d WTA %.2f\n", 
                        getClk(), pData.id, pData.arrivaltime, pData.runningtime, pcbPointer->remainingTime, pcbPointer->waitingTime, TA,WTA);
                    printf("At time %d process %d finished arr %d total %d remain %d wait %d TA %d WTA %.2f\n", 
                        getClk(), pData.id, pData.arrivaltime, pData.runningtime, pcbPointer->remainingTime, pcbPointer->waitingTime, TA,WTA);
                    sum_WeightedTurnAround+=WTA;
                    WTAArray[pData.id] = WTA;

                    sum_Waiting+=pcbPointer->waitingTime;

                    sum_RunningTime+=pcbPointer->executionTime;
                    
                    
                    
                      
                    //Deallocate memory for the finished process
                    int size = pow(2,ceil(log(pData.sizeP)/log(2)));
                    DeAllocate_memory(pcbPointer->mem_address, size);
                    fprintf(sizeP, "\nAt time %d deallocated %d bytes for process %d from %d to %d\n", getClk(), pData.sizeP, pData.id, pcbPointer->mem_address,pcbPointer->mem_address+size);
                    
                    pData.id=-1;
                }
            }    
            else
            {
                //Fork the process
                printf("\nIam forking a new process time = %d\n", getClk());
                int pid=fork();
                pcbPointer->pid=pid;
                if (pid == -1)
                perror("error in forking");

                else if (pid == 0)
                {
                    printf("\ntesting the fork\n");
                    char buf[100];
                    sprintf(buf,"%d",pData.runningtime);
                    char *argv[] = { "./process.out",buf, 0 };
                    execve(argv[0], &argv[0], NULL);
                }

                pcount--;
                printf("\nAt time %d process %d started arr %d total %d remain %d wait %d\n", getClk(), pData.id, pData.arrivaltime, pData.runningtime, pcbPointer->remainingTime, pcbPointer->waitingTime);
                fprintf(logFile, "At time %d process %d started arr %d total %d remain %d wait %d\n", getClk(), pData.id, pData.arrivaltime, pData.runningtime, pcbPointer->remainingTime, pcbPointer->waitingTime);
                printf("\nI`m setting the alarm for 1 TS\n");
                alarm(1);
                sleep(50);  
                if(pcbPointer->state!=Finished)
                {
                    printf("Not Finished\n");
                    push(&HPF_readyQueue, pData.priority, pData);
                    pcbPointer->executionTime+=1;
                    pcbPointer->waitingTime=getClk()-(pData.arrivaltime)-(pcbPointer->executionTime);
                    pcbPointer->state=Waiting;
                    pcbPointer->remainingTime-=1;
                    fprintf(logFile, "At time %d process %d stopped arr %d total %d remain %d wait %d\n", getClk(), pData.id, pData.arrivaltime, pData.runningtime, pcbPointer->remainingTime, pcbPointer->waitingTime);
                }
                else
                {
                    pcbPointer->executionTime=pData.runningtime;
                    pcbPointer->waitingTime=getClk()-(pData.arrivaltime)-(pcbPointer->executionTime);
                    pcbPointer->remainingTime=0;
                    int TA = getClk()-(pData.arrivaltime);
                    double WTA=(double)TA/pData.runningtime;
                    // fprintf(logFile, "At time %d process %d finished arr %d total %d remain %d wait %d TA %d WTA %.2f\n", 
                    //     getClk(), pData.id, pData.arrivaltime, pData.runningtime, pcbPointer->remainingTime, pcbPointer->waitingTime, TA, WTA);
                    // sum_WTA+=WTA;
                    // sum_Waiting+=pcbPointer->waitingTime;
                    // sum_RunningTime+=pcbPointer->executionTime;
                    fprintf(logFile, "At time %d process %d finished arr %d total %d remain %d wait %d TA %d WTA %.2f\n", 
                        getClk(), pData.id, pData.arrivaltime, pData.runningtime, pcbPointer->remainingTime, pcbPointer->waitingTime, TA,WTA);
                    printf("At time %d process %d finished arr %d total %d remain %d wait %d TA %d WTA %.2f\n", 
                        getClk(), pData.id, pData.arrivaltime, pData.runningtime, pcbPointer->remainingTime, pcbPointer->waitingTime, TA,WTA);
                    sum_WeightedTurnAround+=WTA;
                    WTAArray[pData.id] = WTA;

                    sum_Waiting+=pcbPointer->waitingTime;

                    sum_RunningTime+=pcbPointer->executionTime;
                    
                    
                    

                    //Deallocate memory for the finished process
                    int size = pow(2,ceil(log(pData.sizeP)/log(2)));
                    DeAllocate_memory(pcbPointer->mem_address, size);
                    fprintf(sizeP, "\nAt time %d deallocated %d bytes for process %d from %d to %d\n", getClk(), pData.sizeP, pData.id, pcbPointer->mem_address,pcbPointer->mem_address+size);
                    
                    pData.id=-1;
                        
                }
            }   
        }
}

void Alarm_Handler(int signum)
{
    printf("From the alarm handler\n");
    kill(pcbPointer->pid,SIGSTOP); // The kill() function shall send a signal to a process and the signal here is SIGSTOP which pauses the process
}

void ResumeProcess()
{
    printf("Procss with pid %d\n is resumed",pcbPointer->pid);
    kill(pcbPointer->pid,SIGCONT); //resume processes
    pcbPointer->state=Running; //change the status to running
}

int Allocate_memory(int size)
{
    switch (size)
    {
    case 32:
        for (int i = 0; i < 32; i++)
        {
            if(blocks_32[i]!=-1)
            {
                blocks_32[i]=-1;
                return i*size;
            }

        }


        for (int i = 0; i < 16; i++)
        {
            if(blocks_64[i]!=-1)
            {
                blocks_32[2*i+1]=(2*i+1)*size;
                blocks_64[i]=-1;
                return (2*i)*size;
            }

        }

        for (int i = 0; i < 8; i++)
        {
            if(blocks_128[i]!=-1)
            {
                blocks_64[2*i+1]=(2*i+1)*2*size;
                blocks_32[4*i+1]=(4*i+1)*size;
                blocks_128[i]=-1;
                return (4*i)*size;
            }

        }
        for (int i = 0; i < 4; i++)
        {
            if(blocks_256[i]!=-1)
            {
                blocks_128[2*i+1]=(2*i+1)*4*size;
                blocks_64[4*i+1]=(4*i+1)*2*size; //next available 64 in the array of 64 block 
                blocks_32[8*i+1]=(8*i+1)*size; // this is the next available 32 in array of 32 block
                blocks_256[i]=-1;
                return (8*i)*size;
            }
        }

        printf("\n\n Warning! There's no enough space in memory for this process\n");
        
        break;

    case 64:
        for (int i = 0; i < 16; i++)
        {
            if(blocks_64[i]!=-1)
            {

                blocks_64[i]=-1;
                return (i)*size;
            }
        }
        for (int i = 0; i < 8; i++)
        {
            if(blocks_128[i]!=-1)
            {

                blocks_64[2*i+1]=(2*i+1)*size;
                blocks_128[i]=-1;
                return (2*i)*size;
            }
        }
        for (int i = 0; i < 4; i++)
        {
            if(blocks_256[i]!=-1)
            {

                blocks_128[2*i+1]=(2*i+1)*2*size;
                blocks_64[4*i+1]=(4*i+1)*size;
                blocks_256[i]=-1;
                return (4*i)*size;
            }
        }
        printf("\n\n Warning! There's no enough space in memory for this process\n");
        break;

    case 128:
        for (int i = 0; i < 8; i++)
        {

            if(blocks_128[i]!=-1)
            {
                blocks_128[i]=-1;
                return (i)*size;
            }
        }
        for (int i = 0; i < 4; i++)
        {

            if(blocks_256[i]!=-1)
            {
                blocks_128[2*i+1]=(2*i+1)*size;
                blocks_256[i]=-1;
                return (2*i)*size;
            }
        }
        printf("\n\n Warning! There's no enough space in memory for this process\n");
        break;

    case 256:
        for (int i = 0; i < 4; i++)
        {

            if(blocks_256[i]!=-1)
            {
                blocks_256[i]=-1;
                return (i)*size;
            }
        }
        printf("\n\n Warning! There's no enough space in memory for this process\n");
        break;

    }
}

void DeAllocate_memory(int address, int size)
{
    int ratio=address/size; // index in blocks arrays
    switch (size)
    {
    case 32:


        blocks_32[ratio]=address; // -1 means either I am not used yet or I am used and not avaliable so in deallocation we put the address in the place of deallocation to say that this place is available now

        break;
    
    case 64:
        blocks_64[ratio]=address;

        break;
    
    case 128:

        blocks_128[ratio]=address;

        break;

    case 256:

        blocks_256[ratio]=address;

        break;
    }

    printf("\n [256] Memory Blocks: \n");
    for (int i = 0; i < 4 ; i++) printf("%d\t", blocks_256[i]);

    printf("\n[128] Memory Blocks:\n");
    for (int i = 0; i < 8; i++) printf("%d\t", blocks_128[i]);

    printf("\n[64] Memory Blocks:\n");
    for (int i = 0; i < 16; i++) printf("%d\t", blocks_64[i]);

    printf("\n[32] Memory Blocks:\n");
    for (int i = 0; i < 32; i++) printf("%d\t", blocks_32[i]);

    printf("\n\n");


    Merge(address, size);


    printf("\n [256] Memory Blocks: \n");
    for (int i = 0; i < 4 ; i++) printf("%d\t", blocks_256[i]);

    printf("\n[128] Memory Blocks:\n");
    for (int i = 0; i < 8; i++) printf("%d\t", blocks_128[i]);

    printf("\n[64] Memory Blocks:\n");
    for (int i = 0; i < 16; i++) printf("%d\t", blocks_64[i]);

    printf("\n[32] Memory Blocks:\n");
    for (int i = 0; i < 32; i++) printf("%d\t", blocks_32[i]);

    printf("\n\n");
}

void Merge(int address, int size)
{
    //Implementation of merging memory blocks goes here for deallocation 
    switch (size)
    {
    case 32:
        for (int i = 0; i< 32; i+=2)
        {
          if (blocks_32[i] != -1 && blocks_32[i+1] != -1)
          {
           blocks_64[i/2]= i*size; // in first iteration --> block 64 of 0 is now available 
           blocks_32[i] = -1; // in first iteration --> i =0 and i =1 are merged so they are not available anymore 
           blocks_32[i+1]=-1;
           printf("\nmemory 32 merged successfully\n");
          }
        } 

        for (int i = 0; i < 16; i+=2)
        {
            if (blocks_64[i] != -1 && blocks_64[i+1] != -1)
            {
                blocks_128[i/2] = 2*i*size;
                blocks_64[i] = -1;
                blocks_64[i+1] = -1;
                printf("\nmemory 64 merged successfully\n");
            }
        }

        for (int i = 0; i < 8; i +=2)
        {
            if (blocks_128[i] != -1 && blocks_128[i+1] != -1)
            {
                blocks_256[i/2] = 4*i*size;
                blocks_128[i] = -1;
                blocks_128[i+1] = -1;
                printf("\nmemory 128 merged successfully\n");
            }
        }
        break;
    
    case 64:
        for (int i = 0; i < 16; i+= 2)
        {
            if (blocks_64[i] != -1 && blocks_64[i+1] != -1)
            {
                blocks_128[i/2] = i*size;
                blocks_64[i] = -1;
                blocks_64[i+1] = -1;
                printf("\nmemory 64 merged successfully\n");
            }
        }

          for (int i = 0; i < 8; i +=2)
        {
            if (blocks_128[i] != -1 && blocks_128[i+1] != -1)
            {
                blocks_256[i/2] = 2*i*size;
                blocks_128[i] = -1;
                blocks_128[i+1] = -1;
                printf("\nmemory 128 merged successfully\n");
            }
        }

        break;

    case 128:
        for (int i = 0; i < 8; i +=2)
        {
            if (blocks_128[i] != -1 && blocks_128[i+1] != -1)
            {
                blocks_256[i/2] = i*size;
                blocks_128[i] = -1;
                blocks_128[i+1] = -1;
                printf("\nmemory 128 merged successfully\n");
            }
        }
        break;
    }

}

