#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include<string.h>
#include<errno.h>
//extern int errno;
int main(int argc,char *argv[])
{
    if(argc==1)
    {
        
        int fd1[2],fd2[2];
        pipe(fd1);
        pipe(fd2);
        printf("+++ CSE in supervisor mode: Started\n");
        printf("+++ CSE in supervisor mode: pfd = [%d %d]\n",fd1[0],fd1[1]);
        printf("+++ CSE in supervisor mode: Forking first child in command-input mode\n");
        printf("+++ CSE in supervisor mode: Forking second child in execute mode\n");
        int pid=fork();
        if(pid==0)
        {
           //dup()
           char descriptor[10];
            sprintf(descriptor,"%d",fd1[0]);
            char descriptor2[10];
            sprintf(descriptor2,"%d",fd1[1]);
            char descriptor3[10];
            sprintf(descriptor3,"%d",fd2[0]);
            char descriptor4[10];
            sprintf(descriptor4,"%d",fd2[1]);
           int status=execlp("xterm","xterm","-T","First Child","-e","./CSE","C",descriptor,descriptor2,descriptor3,descriptor4,NULL); 
              perror("execl");
              errno=0;
        }
        else
        {
            int pid2=fork();
            if(pid2==0)
            {
                char descriptor[10];
                sprintf(descriptor,"%d",fd1[0]);
                char descriptor2[10];
                sprintf(descriptor2,"%d",fd1[1]);
                char descriptor3[10];
                sprintf(descriptor3,"%d",fd2[0]);
                char descriptor4[10];
                sprintf(descriptor4,"%d",fd2[1]);
                execlp("xterm","xterm","-T","Second Child","-e","./CSE","E",descriptor,descriptor2,descriptor3,descriptor4,NULL); 
                perror("execl");
                errno=0;
            }
            else
            {
                wait(NULL);
                wait(NULL);
                printf("+++ CSE in supervisor mode: First child terminated\n");
                printf("+++ CSE in supervisor mode: Second child terminated\n");
                exit(0);
            }
        }
    }
    else{
        if(strcmp(argv[1],"C")==0)
        {
            int fd[2],fd2[2];  
            fd[0]=atoi(argv[2]);
            fd[1]=atoi(argv[3]);
            fd2[0]=atoi(argv[4]);
            fd2[1]=atoi(argv[5]);
            char buffer[1000];
            int role=0;
            int outfd = dup(1);
            int infd = dup(0);
            close(1);
            dup(fd[1]);
            //close(fd[0]);
            while(1)
            {
                if(!role)
                {
                    char buffer[1000];
                    for(int i=0;i<1000;i++)buffer[i]='\0';
                    fputs("command> ",stderr);
                    fflush(stderr);
                    fgets(buffer,1000,stdin);
                    for(int i=0;i<1000;i++)
                    {
                        if(buffer[i]=='\n')
                        {
                            buffer[i]='\0';
                            break;
                        }
                    }
                    write(fd[1],buffer,1000);
                    //fputs(buffer,stderr);
                    //printf("%s",buffer);
                    fflush(stdout);
                    if(strcmp(buffer,"exit")==0)
                    {
                        close(fd[1]);
                        close(fd2[0]);
                        exit(0);
                    }
                    if(strcmp(buffer,"swaprole")==0)
                    {
                        role=1;
                        close(1);
                        dup(outfd);
                        close(0);
                        dup(fd2[0]);
                    }
                    for(int i=0;i<1000;i++)buffer[i]='\0';
                }
                else
                {
                    printf("Waiting for command: ");
                    fflush(stdout);
                    char buffer[1000];
                    for(int i=0;i<1000;i++)buffer[i]='\0';
                    read(fd2[0],buffer,1000);
                    //fgets(buffer,1000,stdin);
                    printf("%s\n",buffer);
                    fflush(stdout);
                    if(strcmp(buffer,"exit")==0)
                    {
                        close(fd[1]);
                        close(fd2[0]);
                        exit(0);
                    }
                    if(strcmp(buffer,"swaprole")==0)
                    {
                        role=0;
                        close(0);
                        dup(infd);
                        close(1);
                        dup(fd[1]);
                        continue;
                    }
                    int pid3=fork();
                    if(pid3==0)
                    {
                        //printf("executing command: %s\n",buffer);
                        close(0);
                        dup(infd);
                        char**argslist=(char**)malloc(1000*sizeof(char*));
                        char* temp= (char*)malloc(1000*sizeof(char));
                        int i=0;
                        int j=0;
                        int k=0;
                        argslist[0]=(char*)malloc(1000*sizeof(char));
                        while(buffer[i]!='\0')
                        {
                            if(buffer[i]==' ')
                            {
                                argslist[j][k]='\0';
                                j++;
                                argslist[j]=(char*)malloc(1000*sizeof(char));
                                k=0;
                            }
                            else{
                                argslist[j][k]=buffer[i];
                                k++;
                            }
                            i++;
                        }
                        argslist[j][k]='\0';
                        j++;
                        argslist[j]=NULL;
                        int status=execvp(argslist[0],argslist);
                        if(status==-1)
                        {
                            perror("execvp");
                            errno=0;
                        }
                    }
                    else
                    {
                        wait(NULL);
                    }
                    for(int i=0;i<1000;i++)buffer[i]='\0';
                }
            }

        }
        else{
            int fd[2],fd2[2];  
            fd[0]=atoi(argv[2]);
            fd[1]=atoi(argv[3]);
            fd2[0]=atoi(argv[4]);
            fd2[1]=atoi(argv[5]);
            char buffer[1000];
            int role=1;
            int outfd = dup(1);
            int infd = dup(0);
            close(0);
            dup(fd[0]);
            while(1)
            {
                if(role)
                {
                    char buffer[1000];
                    for(int i=0;i<1000;i++)buffer[i]='\0';
                    printf("Waiting for command: ");
                    fflush(stdout);
                    read(fd[0],buffer,1000);
                    //fgets(buffer,1000,stdin);
                    printf("%s\n",buffer);
                    fflush(stdout);
                    if(strcmp(buffer,"exit")==0)
                    {
                        close(fd[0]);
                        close(fd2[1]);
                        exit(0);
                    }
                    if(strcmp(buffer,"swaprole")==0)
                    {
                        role=0;
                        close(0);
                        dup(infd);
                        close(1);
                        dup(fd2[1]);
                        continue;
                    }
                    int pid3=fork();
                    if(pid3==0)
                    {
                        //printf("executing command: %s\n",buffer);
                        close(0);
                        dup(infd);
                        char**argslist=(char**)malloc(1000*sizeof(char*));
                        char* temp= (char*)malloc(1000*sizeof(char));
                        int i=0;
                        int j=0;
                        int k=0;
                        argslist[0]=(char*)malloc(1000*sizeof(char));
                        while(buffer[i]!='\0')
                        {
                            if(buffer[i]==' ')
                            {
                                argslist[j][k]='\0';
                                j++;
                                argslist[j]=(char*)malloc(1000*sizeof(char));
                                k=0;
                            }
                            else{
                                argslist[j][k]=buffer[i];
                                k++;
                            }
                            i++;
                        }
                        argslist[j][k]='\0';
                        j++;
                        argslist[j]=NULL;
                        
                        int status=execvp(argslist[0],argslist);
                        if(status==-1)
                        {
                            perror("execvp");
                            errno=0;
                        }
                        exit(0);
                    }
                    else
                    {
                        wait(NULL);
                    }
                    for(int i=0;i<1000;i++)buffer[i]='\0';
                }
                else
                {
                    char buffer[1000];
                    for(int i=0;i<1000;i++)buffer[i]='\0'; 
                    fputs("command> ",stderr);
                    fflush(stderr);
                    fgets(buffer,1000,stdin);
                    for(int i=0;i<1000;i++)
                    {
                        if(buffer[i]=='\n')
                        {
                            buffer[i]='\0';
                            break;
                        }
                    }
                    write(fd2[1],buffer,1000);
                    //printf("%s",buffer);
                    fflush(stdout);
                    if(strcmp(buffer,"exit")==0)
                    {
                        close(fd[1]);
                        close(fd2[0]);
                        exit(0);
                    }
                    if(strcmp(buffer,"swaprole")==0)
                    {
                        role=1;
                        close(1);
                        dup(outfd);
                        close(0);
                        dup(fd[0]);
                    }  
                    for(int i=0;i<1000;i++)buffer[i]='\0';           
                }
            }
        }
    }

}