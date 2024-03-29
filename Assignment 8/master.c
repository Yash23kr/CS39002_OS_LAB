#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <stdbool.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <sys/msg.h>
#include <time.h>
struct table_entry{
    int frame_number;
    bool valid;
    int previous_access;
};

struct page_table{
    struct table_entry entries[1000];
};

int gen_bool()
{
    float p = (float)rand()/(float)RAND_MAX;
    if(p<0.01)
    {
        return 1;
    }
    return 0;
}
struct ready_queue_message{
    long mtype;
    int i;
};
int num_processes,frames;
void sig_handler(int signo)
{
    if(signo==SIGINT)
    {
        int page_table_id = shmget(ftok("master.c",1),sizeof(struct page_table)*num_processes,IPC_CREAT|0666);
        struct page_table* page_table = (struct page_table*)shmat(page_table_id,NULL,0);
        int free_frame_list_id = shmget(ftok("master.c",2),sizeof(int)*frames,IPC_CREAT|0666);
        int* free_frame_list = (int*)shmat(free_frame_list_id,NULL,0);
        int process_pages_id = shmget(ftok("master.c",3),sizeof(int)*num_processes,IPC_CREAT|0666);
        int* process_pages = (int*)shmat(process_pages_id,NULL,0);
        int ready_queue_id = msgget(ftok("master.c",4),IPC_CREAT|0666);
        int sched_mmu_id = msgget(ftok("master.c",5),IPC_CREAT|0666);
        int process_mmu_id = msgget(ftok("master.c",6),IPC_CREAT|0666);
        int process_sched_id = msgget(ftok("master.c",7),IPC_CREAT|0666);
        shmdt(page_table);
        shmdt(free_frame_list);
        shmdt(process_pages);
        shmctl(page_table_id,IPC_RMID,NULL);
        shmctl(free_frame_list_id,IPC_RMID,NULL);
        shmctl(process_pages_id,IPC_RMID,NULL);
        msgctl(ready_queue_id,IPC_RMID,NULL);
        msgctl(sched_mmu_id,IPC_RMID,NULL);
        msgctl(process_mmu_id,IPC_RMID,NULL);
        msgctl(process_sched_id,IPC_RMID,NULL);
        exit(0);
    }
}
int main(int argc,char* argv[])
{
    if(argc!=4)
    {
        printf("Usage: master k m f\n");
        exit(0);
    }
    srand(time(0));
    signal(SIGINT,sig_handler);
    num_processes = atoi(argv[1]);
    int max_pages = atoi(argv[2]);
    frames = atoi(argv[3]);
    int page_table_id = shmget(ftok("master.c",1),sizeof(struct page_table)*num_processes,IPC_CREAT|0666);
    struct page_table* page_table = (struct page_table*)shmat(page_table_id,NULL,0);
    for(int i=0;i<num_processes;i++)
    {
        for(int j=0;j<max_pages;j++)
        {
            page_table[i].entries[j].valid = false;
            page_table[i].entries[j].frame_number = -1;
            page_table[i].entries[j].previous_access = -1;
        }
    }
    int free_frame_list_id = shmget(ftok("master.c",2),sizeof(int)*frames,IPC_CREAT|0666);
    int* free_frame_list = (int*)shmat(free_frame_list_id,NULL,0);
    for(int i=0;i<frames;i++)
    {
        free_frame_list[i] = -1;
    }
    int process_pages_id = shmget(ftok("master.c",3),sizeof(int)*num_processes,IPC_CREAT|0666);
    int* process_pages = (int*)shmat(process_pages_id,NULL,0);
    for(int i=0;i<num_processes;i++)
    {
        process_pages[i] = 0;
    }
    int ready_queue_id = msgget(ftok("master.c",4),IPC_CREAT|0666);
    int sched_mmu_id = msgget(ftok("master.c",5),IPC_CREAT|0666);
    int process_mmu_id = msgget(ftok("master.c",6),IPC_CREAT|0666);
    int process_sched_id = msgget(ftok("master.c",7),IPC_CREAT|0666);
    printf("Process_sched_id = %d\n",process_sched_id);
    int mmupid = fork();
    if(mmupid==0)
    {
        char* args[10];
        args[0] = (char*)malloc(10*sizeof(char));
        strcpy(args[0],"./mmu");
        args[1] = (char*)malloc(10*sizeof(char));
        sprintf(args[1],"%d",sched_mmu_id);
        args[2] = (char*)malloc(10*sizeof(char));
        sprintf(args[2],"%d",process_mmu_id);
        args[3] = (char*)malloc(10*sizeof(char));
        sprintf(args[3],"%d",page_table_id);
        args[4] = (char*)malloc(10*sizeof(char));
        sprintf(args[4],"%d",free_frame_list_id);
        args[5] = (char*)malloc(10*sizeof(char));
        sprintf(args[5],"%d",process_pages_id);
        args[6] = (char*)malloc(10*sizeof(char));
        sprintf(args[6],"%d",frames);
        args[7]=(char*)malloc(10*sizeof(char));
        sprintf(args[7],"%d",num_processes);
        args[8] = NULL;
        int retval = execvp(args[0],args);
        if(retval==-1)
        {
            perror("execvp mmu");
            exit(0);
        }
    }
    int sched_pid = fork();
    if(sched_pid==0)
    {
        char*args[10];
        args[0] = (char*)malloc(10*sizeof(char));
        strcpy(args[0],"./sched");
        args[1] = (char*)malloc(10*sizeof(char));
        sprintf(args[1],"%d",ready_queue_id);
        args[2] = (char*)malloc(10*sizeof(char));
        sprintf(args[2],"%d",sched_mmu_id);
        args[3] = (char*)malloc(10*sizeof(char));
        sprintf(args[3],"%d",process_sched_id);
        args[4] = NULL;
        int retval = execvp(args[0],args);
        if(retval==-1)
        {
            perror("execvp sched");
            exit(0);
        }
    }

    char** reference_strings[num_processes];
    int reference_lengths[num_processes];
    int total_pages[num_processes];
    int sum=0;
    for(int i=0;i<num_processes;i++)
    {
        int process_pages = rand()%max_pages+1;
        int len = rand()%(8*process_pages+1)+2*process_pages; 
        total_pages[i] = process_pages;
        reference_lengths[i] = len;
        sum+=process_pages;
        reference_strings[i] = (char**)malloc(len*sizeof(char*));
        for(int j=0;j<len;j++)
        {
            reference_strings[i][j] = (char*)malloc(10*sizeof(char));
            if(gen_bool())
            {
                sprintf(reference_strings[i][j],"%d",rand()%process_pages+process_pages);
            }
            else
            {
                sprintf(reference_strings[i][j],"%d",rand()%process_pages);
            }
            //printf("%s\n",reference_strings[i][j]);
        }
    }
    int cur=0;
    for(int i=0;i<num_processes;i++)
    {
        int temp = ((float)total_pages[i]/(float)sum)*(float)frames;
        printf("i=%d,temp=%d\n",i,temp);
        while(temp)
        {
            free_frame_list[cur] = i;
            cur++;
            temp--;
        }
    }
    for(int i=0;i<num_processes;i++)
    {
        process_pages[i] = total_pages[i];
    }
    for(int i=0;i<num_processes;i++)
    {
        usleep(250000);
        int process_pages = total_pages[i];
        int len = reference_lengths[i];
        printf("Process %d, len %d, process_pages %d\n",i,len,process_pages);
        char* args[10+len];
        args[0] = (char*)malloc(10*sizeof(char));
        strcpy(args[0],"./process");
        args[1] = (char*)malloc(10*sizeof(char));
        sprintf(args[1],"%d",i);
        args[2] = (char*)malloc(10*sizeof(char));
        sprintf(args[2],"%d",ready_queue_id);
        args[3] = (char*)malloc(10*sizeof(char));
        sprintf(args[3],"%d",process_sched_id);
        args[4] = (char*)malloc(10*sizeof(char));
        sprintf(args[4],"%d",process_mmu_id);
        for(int j=0;j<len;j++)
        {
            args[5+j] = (char*)malloc(10*sizeof(char));
            strcpy(args[5+j],reference_strings[i][j]);
            //printf("%s\n",args[5+i]);
            //printf("%lld ",args[5+j]);
        }
        // printf("\n");
        // for(int i=0;i<6+len;i++)
        // {
        //     printf("%lld ",args[i]);
        // }
        // printf("\n");
        args[5+len] = NULL;
        int pid = fork();
        if(pid==0)
        {
            int retval = execvp(args[0],args);
            if(retval==-1)
            {
                perror("execvp process");
                exit(0);
            }
        }
        struct ready_queue_message rq;
        rq.mtype = 1;
        rq.i = i;
        int retval = msgsnd(ready_queue_id,&rq,sizeof(int),0);
        printf("Message sent,retval = %d,ready_queue_id = %d\n",retval,ready_queue_id);
        if(retval==-1)
        {
            perror("msgsnd master");
            exit(0);
        }
    }
    //sleep(300);
    waitpid(mmupid,0,0);
    kill(sched_pid,SIGKILL);
    shmdt(page_table);
    shmdt(free_frame_list);
    shmdt(process_pages);
    shmctl(page_table_id,IPC_RMID,NULL);
    shmctl(free_frame_list_id,IPC_RMID,NULL);
    shmctl(process_pages_id,IPC_RMID,NULL);
    msgctl(ready_queue_id,IPC_RMID,NULL);
    msgctl(sched_mmu_id,IPC_RMID,NULL);
    msgctl(process_mmu_id,IPC_RMID,NULL);
    msgctl(process_sched_id,IPC_RMID,NULL);
    return 0;
}