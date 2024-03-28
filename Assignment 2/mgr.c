#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include<string.h>
struct process{
    int pid;
    int pgid;
    char* status;
    char* name;
};
int curpid=0;
int cursize=0;
struct process p[11];
int curpointer;
void stophandler()//Handling stop
{
    if(curpid)//If a process is currently running, stop it
    {
        //printf("%d\n",curpid);
        kill(curpid,SIGINT);
        sprintf(p[curpointer].status,"terminated");
        curpid=0;
        printf("\n");
    }
    else
    {
        printf("\nmgr> ");
        fflush(stdout);
    }
}
void suspendhandler()//Handling suspend
{
    if(curpid)//If a process is currently running, suspend it
    {
        kill(curpid,SIGTSTP);
        sprintf(p[curpointer].status,"suspended");
        curpid=0;
        printf("\n");
    }
    else
    {
        printf("\nmgr> ");
        fflush(stdout);
    }
}
void childhandler()//Handling child
{
    //printf("Hello\n");
    if(curpid)
    sprintf(p[curpointer].status,"terminated");
    curpid=0;
}
int main()
{
    
    p[0].pid=getpid();//Initialising the process table with the entry of mgr
    p[0].pgid=getpgid(getpid());
    p[0].status=(char*)malloc(100*sizeof(char));
    sprintf(p[0].status,"self");
    p[0].name=(char*)malloc(100*sizeof(char));
    sprintf(p[0].name,"mgr");
    signal(SIGINT,stophandler);
    signal(SIGTSTP,suspendhandler);
    //signal(SIGCHLD, childhandler);
    cursize=1;
    while(1)
    {
        char buffer[100];
        printf("mgr> ");
        scanf("%s",buffer);//Scanning the command
        if(strcmp(buffer,"h")==0)//Printing help
        {
            printf("\tCommand : Action\n");
            printf("\tc : Continue a suspended job\n");
            printf("\th : Print this help message\n");
            printf("\tk : Kill a suspended job\n");
            printf("\tp : Print the process table\n");
            printf("\tq : Quit\n");
            printf("\tr : Run a new job\n");

        }
        else if(strcmp(buffer,"q")==0)//Quitting
        {
            for(int i=0;i<cursize;i++)//Killing all the suspended jobs
            {
                if(strcmp(p[i].status,"suspended")==0)
                {
                    kill(p[i].pid,SIGINT);
                }
            }
            exit(0);
        }
        else if(strcmp("p",buffer)==0)//Printing the process table
        {
            printf("Index\tPID\tPGID\tSTATUS\t\tNAME\n");
            //printf("0\t%d\t%d\t%s\t%s\n",p[0].pid,p[0].pgid,p[0].status,p[0].name);
            for(int i=0;i<cursize;i++)
            {
                printf("%d\t%d\t%d\t%s",i,p[i].pid,p[i].pgid,p[i].status);
                if(strcmp(p[i].status,"self")==0)
                {
                    printf("\t\t%s\n",p[i].name);
                }
                else if(strcmp(p[i].status,"killed")==0)
                {
                    printf("\t\t%s\n",p[i].name);
                }
                else
                {
                    printf("\t%s\n",p[i].name);
                }
            }
        }
        else if(strcmp("r",buffer)==0)//Running a new job
        {
            if(cursize==11)
            {
                printf("Maximum size reached\n");
                exit(0);
            }
            //printf("Hello\n");
            int temp=(rand())%26;
            int pid=fork();
            curpid=pid;//Creating process table entry
            p[cursize].pid=pid;
            p[cursize].pgid=pid;
            p[cursize].status=(char*)malloc(100*sizeof(char));
            sprintf(p[cursize].status,"running");
            p[cursize].name=(char*)malloc(100*sizeof(char));
            curpointer=cursize;
            
            sprintf(p[cursize].name,"job %c",temp+'A');
            cursize++;
            //printf("pid=%d\n",pid);
            if(pid==0)//Child process runs the job
            {
                setpgid(0,0);
                char arg[2];
                arg[0]='A'+temp;
                arg[1]='\0';
                //printf("%c\n",arg);
                execlp("./job","./job",arg,NULL);
            }
            else{
                //while(curpid);
                waitpid(pid,NULL,WUNTRACED);//Wait till the job stops
                if(curpid)
                {
                    sprintf(p[curpointer].status,"terminated");
                    curpid=0;
                }
            }
        }
        else if(strcmp("c",buffer)==0)//Continue
        {
            int arr[11];
            int count=0;
            for(int i=0;i<cursize;i++)
            {
                if(strcmp(p[i].status,"suspended")==0)
                {
                    arr[count]=i;
                    count++;
                }
            }
            if(count==0)
            {
                continue;
            }
            printf("Jobs available: ");
            for(int i=0;i<count;i++)
            {
                printf("%d ",arr[i]);
            }
            int start;
            printf("Select process to start: ");
            scanf("%d" ,&start);
            int exist=0;
            for(int i=0;i<count;i++)
            {
                if(arr[i]==start)
                {
                    exist=1;
                    break;
                }
            }
            if(!exist)
            {
                printf("Invalid query\n");
                continue;
            }
            curpointer=start;
            curpid=p[start].pid;
            int pid=curpid;
            kill(p[start].pid,SIGCONT);
            //while(curpid);
            waitpid(pid,NULL,WUNTRACED);
            if(curpid)//Job goes to completion
            {
                sprintf(p[curpointer].status,"terminated");
                curpid=0;
            }
        }
        else if(strcmp("k",buffer)==0)//Kill
        {
            int arr[11];
            int count=0;
            for(int i=0;i<cursize;i++)
            {
                if(strcmp(p[i].status,"suspended")==0)
                {
                    arr[count]=i;
                    count++;
                }
            }
            if(count==0)
            {
                continue;
            }
            printf("Jobs available: ");
            for(int i=0;i<count;i++)
            {
                printf("%d ",arr[i]);
            }
            int start;
            printf("Select process to kill: ");
            scanf("%d" ,&start);
            int exist=0;
            for(int i=0;i<count;i++)
            {
                if(arr[i]==start)
                {
                    exist=1;
                    break;
                }
            }
            if(!exist)
            {
                printf("Invalid query\n");
                continue;
            }
            kill(p[start].pid,SIGKILL);//Kill and update status
            sprintf(p[start].status,"killed");
        }
        else{
            printf("Invalid command\n");
        }
    }
}