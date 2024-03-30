#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <stdbool.h>
#include <signal.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
struct table_entry{
    int frame_number;
    bool valid;
    int previous_access;
};

struct page_table{
    struct table_entry entries[1000];
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

struct sched_mmu_message{
    long mtype;
    int i;
};
int timer=0;

int main(int argc,char* argv[])
{
    int sched_mmu_id = atoi(argv[1]);
    int process_mmu_id = atoi(argv[2]);
    int page_table_id = atoi(argv[3]);
    struct page_table* pagetable = (struct page_table*)shmat(page_table_id,NULL,0);
    int free_frame_list_id = atoi(argv[4]);
    int* free_frame_list = (int*)shmat(free_frame_list_id,NULL,0);
    int process_pages_id = atoi(argv[5]);
    int* process_pages = (int*)shmat(process_pages_id,NULL,0);
    int frames = atoi(argv[6]);
    int num_processes = atoi(argv[7]);
    int fd = open("result.txt",O_CREAT|O_WRONLY,0666);
    while(num_processes)
    {
        struct process_mmu_message message;
        int retval = msgrcv(process_mmu_id,&message,sizeof(message)-sizeof(long),3,0);
        if(retval==-1)
        {
            perror("Error in receiving message");
        }
        //printf("Message received in mmu, i=%d,page_number=%d\n",message.i,message.page_number);
        int index = message.i;
        int page_number = message.page_number;
        if(page_number>=process_pages[index]||(page_number<0&&page_number!=-9))
        {
            //printf("INVALID PAGE NUMBER\n");
            printf("Invalid page reference: (%d,%d)\n",index,page_number);
            char buffer[100];
            sprintf(buffer,"Invalid page reference: (%d,%d)\n",index,page_number);
            write(fd,buffer,strlen(buffer));
            struct mmu_process_message response;
            response.mtype = 2;
            response.frame_number = -2;
            int retval = msgsnd(process_mmu_id,&response,sizeof(response.frame_number),0);
            if(retval == -1)
            {
                perror("Error in sending message to process");
            }
            struct sched_mmu_message sched_message;
            sched_message.mtype = 1;
            sched_message.i = 2;
            retval = msgsnd(sched_mmu_id,&sched_message,sizeof(sched_message.i),0);
            if(retval==-1)
            {
                perror("Error in sending message to sched");
            }
            num_processes--;
            for(int i=0;i<frames;i++)
            {
                if(free_frame_list[i]==index)
                {
                    free_frame_list[i]=-1;
                }
            }
            continue;
        }
        if(page_number==-9)
        {
            for(int i=0;i<frames;i++)
            {
                if(free_frame_list[i]==index)
                {
                    free_frame_list[i]=-1;
                }
            }
            num_processes--;
            struct sched_mmu_message sched_message;
            sched_message.mtype = 1;
            sched_message.i = 2;
            int retval = msgsnd(sched_mmu_id,&sched_message,sizeof(sched_message.i),0);
            if(retval==-1)
            {
                perror("Error in sending message to sched");
            }
            continue;
        }
        if(pagetable[index].entries[page_number].valid)
        {
            //printf("Page already in memory\n");
            printf("Page access: (%d,%d,%d)\n",timer,index,page_number);
            char buffer[100];
            sprintf(buffer,"Page access: (%d,%d,%d)\n",timer,index,page_number);
            write(fd,buffer,strlen(buffer));
            pagetable[index].entries[page_number].previous_access = timer;
            timer++;
            struct mmu_process_message response;
            response.mtype = 2;
            response.frame_number = pagetable[index].entries[page_number].frame_number;
            int retval = msgsnd(process_mmu_id,&response,sizeof(response.frame_number),0);
            if(retval==-1)
            {
                perror("Error in sending message to process");
            }
            continue;
        }
        printf("Page fault: (%d,%d)\n",index,page_number);
        char buffer[100];
        sprintf(buffer,"Page fault: (%d,%d)\n",index,page_number);
        write(fd,buffer,strlen(buffer));
        int alloted_frames = 0;
        for(int i=0;i<frames;i++)
        {
            if(free_frame_list[i]==index)
            {
                alloted_frames++;
            }
        }
        for(int i=0;i<process_pages[index];i++)
        {
            if(pagetable[index].entries[i].valid)
            {
                alloted_frames--;
            }
        }
        if(alloted_frames)
        {
            //printf("Process has free frames alloted to it\n");
            pagetable[index].entries[page_number].valid=true;
            pagetable[index].entries[page_number].previous_access = timer;
            int i=0;
            for(;i<frames;i++)
            {
                int used = 0;
                if(free_frame_list[i]==index)
                {
                    for(int j=0;j<process_pages[index];j++)
                    {
                        if(pagetable[index].entries[j].valid)
                        {
                            if(pagetable[index].entries[j].frame_number==i)
                            {
                                used = 1;
                                break;
                            }
                        }
                    }
                    if(!used)break;
                }
            }
            pagetable[index].entries[page_number].frame_number = i;
            struct mmu_process_message response;
            response.mtype = 2;
            response.frame_number = -1;
            int retval = msgsnd(process_mmu_id,&response,sizeof(response.frame_number),0);
            if(retval==-1)
            {
                perror("Error in sending message to process");
            }
            struct sched_mmu_message msg2;
            msg2.mtype = 1;
            msg2.i = 1;
            msgsnd(sched_mmu_id,&msg2,sizeof(msg2.i),0);
            continue;
        }
        int found=0;
        for(int i=0;i<frames;i++)
        {
            if(free_frame_list[i]==-1)
            {
                //printf("Stealing a global free frame\n");
                free_frame_list[i] = index;
                pagetable[index].entries[page_number].valid = true;
                pagetable[index].entries[page_number].previous_access = timer;
                pagetable[index].entries[page_number].frame_number = i;
                struct mmu_process_message response;
                response.mtype = 2;
                response.frame_number = -1;
                int retval = msgsnd(process_mmu_id,&response,sizeof(response.frame_number),0);
                if(retval==-1)
                {
                    perror("Error in sending message to process");
                }
                struct sched_mmu_message msg2;
                msg2.mtype = 1;
                msg2.i = 1;
                retval = msgsnd(sched_mmu_id,&msg2,sizeof(msg2.i),0);
                if(retval==-1)
                {
                    perror("Error in sending message to sched");
                }
                found=1;
                break;
            }
        }
        if(found)continue;

        //printf("No free frames available, replacing page\n");
        int replacement_index = -1;
        int min = 1000000;
        for(int i=0;i<process_pages[index];i++)
        {
            if(pagetable[index].entries[i].valid)
            {
                if(pagetable[index].entries[i].previous_access<min)
                {
                    min = pagetable[index].entries[i].previous_access;
                    replacement_index = i;
                }
            }
        }
        if(replacement_index==-1)
        {
            printf("No frames have been allocated to the process yet\n");
            write(fd,"No frames have been allocated to the process yet\n",strlen("No frames have been allocated to the process yet\n"));
            struct mmu_process_message response;
            response.mtype = 2;
            response.frame_number = -1;
            int retval = msgsnd(process_mmu_id,&response,sizeof(response.frame_number),0);
            if(retval==-1)
            {
                perror("Error in sending message to process");
            }
            struct sched_mmu_message msg2;
            msg2.mtype = 1;
            msg2.i = 1;
            retval = msgsnd(sched_mmu_id,&msg2,sizeof(msg2.i),0);
            if(retval==-1)
            {
                perror("Error in sending message to sched");
            }
            continue;
        }
        printf("Replacing page: (%d,%d)\n",index,replacement_index);
        pagetable[index].entries[page_number].valid = true;
        pagetable[index].entries[page_number].previous_access = timer;
        pagetable[index].entries[page_number].frame_number = pagetable[index].entries[replacement_index].frame_number;
        pagetable[index].entries[replacement_index].valid = false;
        struct mmu_process_message response;
        response.mtype = 2;
        response.frame_number = -1;
        retval = msgsnd(process_mmu_id,&response,sizeof(response.frame_number),0);
        if(retval==-1)
        {
            perror("Error in sending message to process");
        }
        struct sched_mmu_message msg2;
        msg2.mtype = 1;
        msg2.i = 1;
        retval = msgsnd(sched_mmu_id,&msg2,sizeof(msg2.i),0);
        if(retval==-1)
        {
            perror("Error in sending message to sched");
        }

    }

}