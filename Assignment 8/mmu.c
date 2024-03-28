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

int main(int argc,char* argv[])
{
    int sched_mmu_id = atoi(argv[1]);
    int process_mmu_id = atoi(argv[2]);
    int page_table_id = atoi(argv[3]);
    int free_frame_list_id = atoi(argv[4]);
    int process_pages_id = atoi(argv[5]);
    while(1)
    {
        
    }

}