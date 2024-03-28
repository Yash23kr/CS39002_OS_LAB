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
    for(int i=0;i<argc;i++)
    {
        printf("%s\n",argv[i]);
    }
}