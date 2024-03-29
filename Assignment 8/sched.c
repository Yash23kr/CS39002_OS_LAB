#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <stdbool.h>
#include <signal.h>
struct table_entry{
    int frame_number;
    bool valid;
    int previous_access;
};

struct page_table{
    struct table_entry entries[1000];
};

struct ready_queue_message{
    long mtype;
    int i;
};

struct process_sched_message{
    long mtype;
};

struct sched_mmu_message{
    long mtype;
    int i;
};
int timestamp=0;
int main(int argc,char* argv[])
{
    // for(int i=0;i<argc;i++)
    // {
    //     printf("%s\n",argv[i]);
    // }
    int ready_queue_id = atoi(argv[1]);
    int sched_mmu_id = atoi(argv[2]);
    int process_sched_id = atoi(argv[3]);
    while(1)
    {
        struct ready_queue_message message;
        int retval = msgrcv(ready_queue_id,&message,sizeof(message.i),0,0);
        printf("Message received from ready queue, i=%d\n",message.i);
        fflush(stdout);
        if(retval==-1)
        {
            perror("msgrcv");
            exit(1);
        }
        struct process_sched_message sched_message;
        sched_message.mtype = message.i+1;
        printf("Sending message to process_sched_id = %d,sched_message.mtype = %ld\n",process_sched_id,sched_message.mtype);
        retval = msgsnd(process_sched_id,&sched_message,0,0);
        if(retval==-1)
        {
            perror("msgsnd");
            printf("process_sched_id = %d,sched_message.mtype = %ld\n",process_sched_id,sched_message.mtype);
            exit(1);
        }
        struct sched_mmu_message mmu_message;
        retval = msgrcv(sched_mmu_id,&mmu_message,sizeof(mmu_message.i),0,0);
        if(retval==-1)
        {
            perror("msgrcv on sched_mmu_id");
            exit(1);
        }
        if(mmu_message.i==1)
        {
            msgsnd(ready_queue_id,&message,sizeof(message.i),0);
        }
        
    }
}