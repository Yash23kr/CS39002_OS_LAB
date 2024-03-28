
#include <stdio.h>
#include<stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include<unistd.h>
//define wait and signal operation as macros
#define P(s) semop(s, &pop, 1)  /* pop is the structure we pass for doing
				   the P(s) operation */
#define V(s) semop(s, &vop, 1)  /* vop is the structure we pass for doing
				   the V(s) operation */
int main()
{
    FILE* fp=fopen("graph.txt","r");//open file
    struct sembuf pop, vop ;
    pop.sem_num = vop.sem_num = 0;
	pop.sem_flg = vop.sem_flg = 0;
	pop.sem_op = -1 ; vop.sem_op = 1 ;
    int n;
    fscanf(fp,"%d",&n);//read n
    //Create shared memory for adjacency matrix, topo array and index
    int shmid;
    shmid=shmget(ftok("graph.txt",65),(n*n)*sizeof(int),IPC_CREAT|0666);
    if(shmid==-1)
    {
        perror("shmget: ");
        return 0;
    }
    int shmid2=shmget(ftok("graph.txt",66),sizeof(int),IPC_CREAT|0666);
    if(shmid2==-1)
    {
        perror("shmget: ");
        return 0;
    }
    int* idx=shmat(shmid2,0,0);
    *idx=0;
    int shmid3=shmget(ftok("graph.txt",67),n*sizeof(int),IPC_CREAT|0666);
    if(shmid3==-1)
    {
        perror("shmget: ");
        return 0;
    }
    int* A=shmat(shmid3,0,0);
    //Initialize topo array to -1
    for(int i=0;i<n;i++)
    { 
        A[i]=-1;
        //printf("%d\n",i);
    }
    int* arr=shmat(shmid,0,0);
    int index=0;
    //Read adjacency matrix
    for(int i=0;i<n;i++)
    {
        for(int j=0;j<n;j++)
        {
            fscanf(fp,"%d",&arr[index]);
            index++;
        }
    }
    //Create semaphores and set their values
    int semid=semget(ftok("graph.txt",65),n,IPC_CREAT|0666);
    int ntfy=semget(ftok("graph.txt",66),1,IPC_CREAT|0666);
    int mutex=semget(ftok("graph.txt",67),1,IPC_CREAT|0666);
    for(int i=0;i<n;i++)
    {
        semctl(semid,i,SETVAL,0);
    }
    semctl(ntfy,0,SETVAL,0);
    semctl(mutex,0,SETVAL,1);
    printf("+++Boss: Setup done\n");
    //Wait for all workers to return
    for(int i=0;i<n;i++)
    {
        P(ntfy);
        //printf("%d\n",i);
    }
    printf("+++All workers returned\n");
    //Remove semaphores and shared memory
    semctl(semid,0,IPC_RMID,0);
    semctl(ntfy,0,IPC_RMID,0);
    semctl(mutex,0,IPC_RMID,0);
    shmctl(shmid,IPC_RMID,0);
    shmctl(shmid2,IPC_RMID,0);
    shmctl(shmid3,IPC_RMID,0);
    //Check if topological sort is correct
    printf("+++Topological sorting of vertices: ");
    for(int i=0;i<n;i++)
    {
        printf("%d ",A[i]);
    }
    printf("\n");
    int correct=1;
    int indeg[n];
    for(int i=0;i<n;i++)
    {
        indeg[i]=0;
        for(int j=0;j<n;j++)
        {
            if(arr[j*n+i]==1)
            {
                indeg[i]++;
            }
        }
    }
    for(int i=0;i<n;i++)
    {
        int node=A[i];
        if(indeg[node]!=0)
        {
            correct=0;
            printf("%d %d\n",node,indeg[node]);
        }
        for(int j=0;j<n;j++)
        {
            if(arr[node*n+j]==1)
            {
                indeg[j]--;
            }
        }
    }
    //Print result and exit
    if(correct)
    {
        printf("+++Boss: Topological sort correct\n");
        printf("+++Boss: Exiting\n");
        exit(0);
    }
    else{
        printf("+++Boss: Topological sort incorrect\n+++Boss exiting\n");
        exit(0);
    }
}
