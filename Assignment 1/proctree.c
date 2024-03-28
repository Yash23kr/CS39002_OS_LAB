#include <stdlib.h>
#include<stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/ipc.h>

int main(int argc,char*argv[])
{
    if(argc<=1)
    {
        printf("Run with a node name\n");
        exit(0);
    }
    // for(int i=0;i<argc;i++)
    // {
    //     printf("%s\n",argv[i]);
    // }
    int level=0;//Indentation level
    char*city;//City name from the argument
    //Extract arguments
    if(argc==2)
    {
        city=argv[1];
    }
    if(argc==3)
    {
        city=argv[1];
        level=atoi(argv[2]);
    }
    //printf("City: %s\n",city);
    FILE*fp=fopen("treeinfo.txt","r");//File open
    char*line=(char*)malloc(500*sizeof(char));
    int found=0;
    //Search each line of the file for the city name
    while(!feof(fp))
    {
        fgets(line,500*sizeof(char),fp);
        char*node=(char*)malloc(500*sizeof(char));
        int i=0;
        while(line[i]!=' ')
        {
            node[i]=line[i];
            i++;
        }
        node[i]='\0';
        if(strcmp(node,city)==0)
        {
            found=1;
            break;
        }
        //printf("City: %s\n",city);
    }
    //If city does not exist
    if(!found)
    {
        printf("City %s not found\n",city);
        exit(0);
    }
    //Print indent
    for(int i=0;i<level;i++)
    {
        printf("\t");
    }
    printf("%s (%d)\n",city,getpid());
    int i=0;
    while(line[i]!=' ')
    {
        i++;
    }
    i++;
    int temp=i;
    //printf("%s\n",line);
    char* children=(char*)malloc(500*sizeof(char));//Extract Number of children
    while(line[i]>='0'&line[i]<='9')
    {
        children[i-temp]=line[i];
        i++;
    }
    children[i-temp]='\0';
    i++;
    int child=atoi(children);
    //printf("%d\n", child);
    int pid[child];
    //Process each child
    for(int j=0;j<child;j++)
    {
        char*childname=(char*)malloc(500*sizeof(char));
        int temp=i;
        //Extract child name
        while((line[i]>='a'&&line[i]<='z')||(line[i]>='A'&&line[i]<='Z'))
        {
            childname[i-temp]=line[i];
            i++;
        }
        childname[i-temp]='\0';
        i++;
        //printf("%s\n", childname);
        pid[j]=fork();//Fork
        if(pid[j])
        {
           // printf("Hello\n");
           wait(NULL);//Wait till this branch prints out
        }
        else
        {
            //Child process, call exec on the proctree executable
            char*args[4];
            args[0]=(char*)malloc(500*sizeof(char));
            strcpy(args[0],"./proctree");
            args[1]=(char*)malloc(500*sizeof(char));
            strcpy(args[1],childname);
            args[2]=(char*)malloc(500*sizeof(char));
            sprintf(args[2],"%d",level+1);
            args[3]=NULL;
            //printf("%s\n",args[2]);
            //printf("Childname: %s\n", childname);
            char* levelarg=(char*)malloc(500*sizeof(char));
            sprintf(levelarg,"%d",level+1);
            int status=execl("./proctree","./proctree",childname,levelarg,NULL);
            if(status==-1)
            {
                printf("Error\n");

            }
        }
    }
    exit(0);
}