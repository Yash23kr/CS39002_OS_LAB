
#include <stdio.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include<stdlib.h>
#include<unistd.h>
//Define wait and signal operation as macros
#define P(s) semop(s, &pop, 1)  /* pop is the structure we pass for doing
				   the P(s) operation */
#define V(s) semop(s, &vop, 1)  /* vop is the structure we pass for doing
				   the V(s) operation */
int main(int argc,char**argv)
{
    if(argc!=3)
    {
        printf("Usage: ./worker <n> <i>\n");
        return 0;
    }
    //Read n and i from command line
    int n=atoi(argv[1]);
    int i=atoi(argv[2]);
    //Create pop and vop structures
    struct sembuf pop, vop ;
    pop.sem_num = vop.sem_num = i;
	pop.sem_flg = vop.sem_flg = 0;
	pop.sem_op = -1 ; vop.sem_op = 1 ;
    //attach shared memory for adjacency matrix, topo array and index
    int shmid;
    shmid=shmget(ftok("graph.txt",65),(n*n)*sizeof(int),IPC_CREAT|0666);
    if(shmid==-1)
    {
        perror("shmget: ");
        return 0;
    }
    int* arr=shmat(shmid,0,0);
    int shmid2=shmget(ftok("graph.txt",66),sizeof(int),IPC_CREAT|0666);
    if(shmid2==-1)
    {
        perror("shmget: ");
        return 0;
    }
    int* idx=shmat(shmid2,0,0);
    int shmid3=shmget(ftok("graph.txt",67),n*sizeof(int),IPC_CREAT|0666);
    int* A=shmat(shmid3,0,0);
    int indegree=0;
    //Check indegree and wait for indegree number times
    for(int j=0;j<n;j++)
    {
        if(arr[j*n+i]==1)
        {
            indegree++;
        }
    }
    int semid=semget(ftok("graph.txt",65),n,IPC_CREAT|0666);
    int ntfy=semget(ftok("graph.txt",66),1,IPC_CREAT|0666);
    int mutex=semget(ftok("graph.txt",67),1,IPC_CREAT|0666);
    for(int j=0;j<indegree;j++)
    {
        P(semid);
    }
    pop.sem_num=0;
    vop.sem_num=0;
    //Critical section
    P(mutex);
    A[*idx]=i;
    *idx=*idx+1;
    //Exit section
    //signal the mutex and ntfy to boss
    V(mutex);
    V(ntfy);
    //Signal the semaphores of the children
    for(int j=0;j<n;j++)
    {
        if(arr[i*n+j]==1)
        {
            vop.sem_num=j;
            V(semid);
        }
    }
    //exit
    printf("+++Worker %d: Done\n",i);

}