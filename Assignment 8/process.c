#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <stdbool.h>
#include <signal.h>
#include<unistd.h>
struct table_entry{
    int frame_number;
    bool valid;
    int previous_access;
};

struct page_table{
    struct table_entry entries[1000];
};

struct process_sched_message{
    long mtype;
};

struct process_mmu_message{
    long mtype;
    int i;
    int page_number;
};

struct mmu_process_message{
    long mtype;
    int frame_number;
};

int main(int argc, char* argv[])
{
    int index = atoi(argv[1]);
    int ready_queue_id = atoi(argv[2]);
    int process_sched_id = atoi(argv[3]);
    int process_mmu_id = atoi(argv[4]);
    struct process_sched_message msg;
    int retval = msgrcv(process_sched_id,&msg,0,index+1,0);
    if(retval==-1)
    {
        perror("msgrcv");
        exit(1);
    }
    //printf("Message received in process, process_sched_id = %d,retval = %d\n",process_sched_id,retval);
    fflush(stdout);
    for(int i=5;i<argc;i++)
    {
        struct process_mmu_message msg;
        msg.mtype = 3;
        msg.i = index;
        msg.page_number = atoi(argv[i]);
        //printf("Sending message to process_mmu_id = %d,msg.mtype = %ld,msg.i = %d,msg.page_number = %d\n",process_mmu_id,msg.mtype,msg.i,msg.page_number);
        retval = msgsnd(process_mmu_id,&msg,sizeof(msg)-sizeof(long),0);
        if(retval==-1)
        {
            perror("msgsnd process_mmu_message");
            exit(1);
        }
        struct mmu_process_message msg2;
        retval = msgrcv(process_mmu_id,&msg2,sizeof(msg2.frame_number),2,0);
        if(retval==-1)
        {
            perror("msgrcv mmu_process_message");
            exit(1);
        }
        if(msg2.frame_number==-2)
        {
            //printf("Segfault\n");
            fflush(stdout);
            return 0;
        }
        else if(msg2.frame_number==-1)
        {
                // struct process_sched_message msg;
                // //printf("Going to wait on ready queue\n");
                // int retval = msgrcv(process_sched_id,&msg,0,index+1,0);
                // if(retval==-1)
                // {
                //     perror("msgrcv ready_queue");
                //     exit(1);
                // }
                struct process_sched_message msg;
                //printf("Going to wait on ready queue,process_sched_id = %d,index+1=%d\n",process_sched_id,index+1);
                //fflush(stdout);
                int retval = msgrcv(process_sched_id,&msg,0,index+1,0);
                //printf("Out of ready queue\n");
                //fflush(stdout);
                if(retval==-1)
                {
                    perror("msgrcv");
                    exit(1);
                }
                //printf("Message received in process, process_sched_id = %d,retval = %d\n",process_sched_id,retval);
                fflush(stdout);
                i--;
        }
        else
        {
            //printf("Found page at frame at %d\n",msg2.frame_number);
        }
        //sleep(1);

    }
    struct process_mmu_message msg2;
    msg2.mtype = 3;
    msg2.i = index;
    msg2.page_number = -9;
    //printf("Sending message to process_mmu_id = %d,msg.mtype = %ld,msg.i = %d,msg.page_number = %d\n",process_mmu_id,msg2.mtype,msg2.i,msg2.page_number);
    retval = msgsnd(process_mmu_id,&msg2,sizeof(msg2)-sizeof(long),0);
    if(retval==-1)
    {
        perror("msgsnd process_mmu_message");
        exit(1);
    }

}