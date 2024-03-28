#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
//Setting the flag values
//int verbose=0;
#ifdef VERBOSE
    int verbose=1;
#else
    int verbose=0;
#endif

//int sleep=0;
#ifdef SLEEP
    int sleep2=1;
#else   
    int sleep2=0;
#endif


int main(int argc, char*argv[])
{
    int n,t;
    //Taking input from user
    printf("Enter number of child process to be forked: ");
    scanf("%d",&n);
    printf("Enter the number of items to be produced: ");
    scanf("%d",&t);
    int shmid = shmget(IPC_PRIVATE, 2*sizeof(int), IPC_CREAT | 0777);
    int *buffer = (int *)shmat(shmid, 0, 0);
    buffer[0]=0;
    for(int i=0;i<n;i++)
    {
        int pid=fork();
        //Creating child processes
        if(pid==0)
        {
            //Consumer process loop
            int *buf2=(int*)shmat(shmid,0,0);
            int sum=0;
            int count=0;
            printf("\t\tConsumer %d alive\n",i+1);
            //Consumer process loop
            while(buf2[0]!=-1)
            {
                //Checking if the item is for the consumer
                if(buf2[0]==i+1)
                {
                    //Consuming the item
                    sum+=buf2[1];
                    count++;
                    //Verbose mode
                    if(verbose)
                    {
                        printf("\t\tConsumer %d consumed item %d\n",i+1,buf2[1]);
                    }
                    buf2[0]=0;
                }
            }
            //Printing the number of items consumed by the consumer
            printf("\t\tConsumer %d has read %d items with sum %d\n",i+1,count,sum);
            fflush(stdout);
            shmdt(buf2);
            exit(0);
        }
    }
    //Producer process loop
    printf("Producer alive\n");
    int consumer_count[n];
    int consumer_sum[n];
    for(int i=0;i<n;i++)
    {
        consumer_count[i]=0;
        consumer_sum[i]=0;
    }
    for(int i=0;i<t;i++)
    {
        //Checking if the buffer is empty
        while(buffer[0]!=0);
        //Producing the item
        int consumer=(rand()%n)+1;
        int item=rand()%900+100;
        //Verbose mode
        if(verbose)
        {
            printf("Producer produced item %d for consumer %d\n",item,consumer);
        }
        buffer[0]=consumer;
        if(sleep2)
        {
            usleep(10);
        }
        buffer[1]=item;
        //Updating the consumer count and sum
        consumer_count[consumer-1]++;
        consumer_sum[consumer-1]+=item;
    }
    //Checking if the buffer is empty
    while(buffer[0]!=0);
    //Setting the buffer to -1 to indicate that the producer has finished producing
    buffer[0]=-1;
    int status;
    for(int i=0;i<n;i++)
    {
        wait(&status);
    }
    //Printing the number of items produced by the producer
    printf("Producer has produced %d items\n",t);
    for(int i=0;i<n;i++)
    {
        printf("%d items for consumer %d with sum %d\n",consumer_count[i],i+1,consumer_sum[i]);
    }
    //Detaching and deleting the shared memory
    shmdt(buffer);
    shmctl(shmid, IPC_RMID, NULL);
    return 0;
}