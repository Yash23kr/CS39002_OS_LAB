
#define _GNU_SOURCE
#include <stdio.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sched.h>
#include "foothread.h"
#include <stdlib.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#define MAXTHREADS 1000
static int numthreads = 1;
static int exit_threads = 0;
static long joinable_threads[MAXTHREADS];
int cur=0;
int exit_semaphore = -1;
int global_mutex = -1;
#define P(s) semop(s, &pop, 1)  
#define V(s) semop(s, &vop, 1) 
struct sembuf pop = {0,-1,0}, vop = {0,1,0};
void foothread_attr_setjointype ( foothread_attr_t * attr , int jointype)
{
    attr->join_type = jointype ;
}
void foothread_attr_setstacksize ( foothread_attr_t * attr , int stacksize)
{
    attr->stack_size = stacksize ;
}

void foothread_create ( foothread_t * tstruct, foothread_attr_t * attrstruct, int (*f)(void *) , void * args) 
{
    int detached_state;
    int stack_size;
    if(attrstruct!=NULL)
    {
        detached_state = attrstruct->join_type;
        stack_size = attrstruct->stack_size;
    }
    else
    {
        detached_state = FOOTHREAD_DETACHED;
        stack_size = FOOTHREAD_DEFAULT_STACK_SIZE;
    }
    if(tstruct)
    {
        tstruct->joinable = detached_state;
        tstruct->stack_size = stack_size;
    }
    if(global_mutex==-1)
    {
        global_mutex = semget(IPC_PRIVATE, 1, 0666 | IPC_CREAT);
        semctl(global_mutex, 0, SETVAL, 1);
    }
    if(exit_semaphore==-1)
    {
        exit_semaphore = semget(IPC_PRIVATE, 1, 0666 | IPC_CREAT);
        semctl(exit_semaphore, 0, SETVAL, 0);
    }
    P(global_mutex);
    if(detached_state==FOOTHREAD_JOINABLE)
    {
        //printf("Thread is joinable\n");
        //Needs a global semaphore
        if(cur>=MAXTHREADS)
        {
            printf("Error: Maximum number of threads reached\n");
            V(global_mutex);
            return;
        }
        numthreads++;
    }
    else
    {
        //printf("Thread is detached\n");
    }
    void* stack = malloc(stack_size);
    //printf("Stack created\n");
    int retval=clone(f,stack+stack_size,CLONE_THREAD|CLONE_SIGHAND|CLONE_VM,args);
    if(detached_state==FOOTHREAD_JOINABLE)
    {
        joinable_threads[cur] = retval;
        cur++;
    }
    V(global_mutex);
 
}
void foothread_exit()
{
    if(global_mutex==-1)
    {
        global_mutex = semget(IPC_PRIVATE, 1, 0666 | IPC_CREAT);
        semctl(global_mutex, 0, SETVAL, 1);
    }
    if(exit_semaphore==-1)
    {
        exit_semaphore = semget(IPC_PRIVATE, 1, 0666 | IPC_CREAT);
        semctl(exit_semaphore, 0, SETVAL, 0);
    }
    P(global_mutex);
    int found=0;
    for(int i=0;i<cur;i++)
    {
        if(joinable_threads[i]==gettid())
        {
            found=1;
            break;
        }
    }
    if(gettid()==getpid())
    {
        found=1;
    }
    if(!found)
    {
        V(global_mutex);
        return;
    }
    exit_threads++;
    if(exit_threads!=numthreads)
    {
        V(global_mutex);
        P(exit_semaphore);
    }
    else
    {
        V(global_mutex);
        for(int i=0;i<numthreads-1;i++)
        {
            V(exit_semaphore);
        }
    }
}
void foothread_mutex_init ( foothread_mutex_t * mutex_attr ) 
{
    mutex_attr->semid = semget(IPC_PRIVATE, 1, 0777 | IPC_CREAT);
    //printf("Semid:%d\n",mutex_attr->semid);
    semctl(mutex_attr->semid, 0, SETVAL, 1);
}
void foothread_mutex_lock ( foothread_mutex_t * mutex_attr ) 
{
    if(global_mutex==-1)
    {
        global_mutex = semget(IPC_PRIVATE, 1, 0666 | IPC_CREAT);
        semctl(global_mutex, 0, SETVAL, 1);
    }
    
    int retval=P(mutex_attr->semid);
    if(retval==-1)
    {
        perror("retval");
    }
    retval=P(global_mutex); 
    if(retval==-1)
    {
        perror("retval");
    }
    // printf("%d\n\n\n\n",retval);
    // printf("Semid:%d\n",mutex_attr->semid);
    // printf("Value of the semaphore:%d\n",semctl(mutex_attr->semid, 0, GETVAL, 0));
    // printf("%d thread owns the mutex now\n",gettid());
    mutex_attr->tid = gettid();
    V(global_mutex);
}
void foothread_mutex_unlock ( foothread_mutex_t * mutex_attr ) 
{
    P(global_mutex);
    if(mutex_attr->tid==gettid())
    {
        int value = semctl(mutex_attr->semid, 0, GETVAL, 0);
        if(value==0)
        {
            //printf("Thread %d unlocking the mutex\n",gettid());
            V(mutex_attr->semid);
        }
        else
        {
            printf("Error: Mutex is already unlocked\n");
        }
    }
    else
    {
        printf("Error: Thread %d does not own the lock\n",gettid());
    }
    V(global_mutex);
}
void foothread_mutex_destroy(foothread_mutex_t * mutex_attr)
{
    semctl(mutex_attr->semid, 0, IPC_RMID, 0);
}

void foothread_barrier_init ( foothread_barrier_t * barrier_attr , int count) 
{
    barrier_attr->semid = semget(IPC_PRIVATE, 1, 0777 | IPC_CREAT);
    semctl(barrier_attr->semid, 0, SETVAL, 0);
    barrier_attr->count = count;
    barrier_attr->orig_count = count;
}

void foothread_barrier_wait(foothread_barrier_t * barrier_attr)
{
    if(global_mutex==-1)
    {
        global_mutex = semget(IPC_PRIVATE, 1, 0666 | IPC_CREAT);
        semctl(global_mutex, 0, SETVAL, 1);
    }
    P(global_mutex);
    {
        barrier_attr->count--;
        //printf("barrier count:%d\n",barrier_attr->count);
        if(barrier_attr->count==0)
        {
            //printf("Waking all processes");
            for(int i=0;i<barrier_attr->orig_count;i++)
            {
                V(barrier_attr->semid);
            }
        }
        else{
            //printf("Going to sleep\n");
            V(global_mutex);
            P(barrier_attr->semid);
            return;
        }
    }
    V(global_mutex);
}
void foothread_barrier_destroy(foothread_barrier_t * barrier_attr)
{
    semctl(barrier_attr->semid, 0, IPC_RMID, 0);
}

