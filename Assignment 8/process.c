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

struct process_sched_message{
    long mtype;
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
    printf("Message received in process, process_sched_id = %d,retval = %d\n",process_sched_id,retval);
    fflush(stdout);
    for(int i=0;i<argc;i++)
    {
        printf("%s\n",argv[i]);
        fflush(stdout);
    }
}