#include "headers.h"

int main(int agrc, char * argv[])
{
    initClk();
    int runningtime;
    printf("\n from process.c file a new process is created\n");

    sscanf(argv[1],"%d",&runningtime);
    printf("Runtime of process is: %d and the clock is %d\n",runningtime,getClk());

    while (clock()/CLOCKS_PER_SEC <  runningtime);
    // If it ended its job 
    printf("\n Process is done. Current clock is: %d \n",getClk());
    destroyClk(false);
    kill(getppid(),SIGUSR1); //send signal to the scheduler when finished
    return 0;
}
