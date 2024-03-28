#define _GNU_SOURCE
#include<stdio.h>
#include<stdlib.h>
#include"foothread.h"
int* num_children;
int* parent;
struct foothread_barrier_t* barriers;
foothread_mutex_t mutex;
int* sum;
int computesum(void* curnode)
{
    int node = (int)curnode;
    //printf("Child %d created,%d\n",node,gettid());
    foothread_barrier_wait(&barriers[node]);
    //printf("Node %d returned from barrier\n",node);
    fflush(stdout);
    //printf("Node %d calculated: %d\n",node,sum[node]);
    foothread_mutex_lock(&mutex);
    if(num_children[node]==0)
    {
        printf("Enter the value of node %d: ",node);
        fflush(stdout);
        scanf("%d",&sum[node]);
    }
    else
    {
        printf("Value of node %d=%d\n",node,sum[node]);
        fflush(stdout);
    }
    sum[parent[node]] += sum[node];
    if(parent[node]!=node)
        printf("%d added %d to parent %d\n",node,sum[node],parent[node]);
    foothread_mutex_unlock(&mutex);
    if(parent[node]==node)
    {
        sum[parent[node]]/=2;
    }
    foothread_barrier_wait(&barriers[parent[node]]);
    //printf("Node %d exiting\n",node);
    foothread_exit();
}
int main()
{
    FILE* fptr = fopen("tree.txt", "r");
    int numnodes;
    foothread_mutex_init(&mutex);
    fscanf(fptr, "%d", &numnodes);
    sum = (int*)malloc(numnodes*sizeof(int));
    for(int i=0;i<numnodes;i++)
    {
        sum[i] = 0;
    }
    num_children = (int*)malloc(numnodes*sizeof(int));
    for(int i=0;i<numnodes;i++)
    {
        num_children[i] = 0;
    }
    parent = (int*)malloc(numnodes*sizeof(int));
    for(int i=0;i<numnodes;i++)
    {
        parent[i] = i;
    }
    barriers = (struct foothread_barrier_t*)malloc(numnodes*sizeof(struct foothread_barrier_t));
    for(int i=0; i<numnodes; i++)
    {
        int child,par;
        fscanf(fptr, "%d %d", &child, &par);
        if(child==par)
        {
            continue;
        }
        num_children[par]++;
        parent[child] = par;
    }
    for(int i=0;i<numnodes;i++)
    {
        foothread_barrier_init(&barriers[i], num_children[i]+1);
    }
    foothread_t* threads = (foothread_t*)malloc(numnodes*sizeof(foothread_t));
    foothread_attr_t* attr = (foothread_attr_t*)malloc(numnodes*sizeof(foothread_attr_t));
    for(int i=0;i<numnodes;i++)
    {
        foothread_attr_setjointype(&attr[i], FOOTHREAD_JOINABLE);
        foothread_attr_setstacksize(&attr[i], 2097152);
    }
    // for(int i=0;i<numnodes;i++)
    // {
    //     printf("%d %d\n",num_children[i],parent[i]);
    // }
    for(int i=0;i<numnodes;i++)
    {
        foothread_create(&threads[i], &attr[i], computesum, (void*)i);
    }
    // for(int i=0;i<numnodes;i++)
    // {
    //     foothread_barrier_destroy(&barriers[i]);
    // }
    // foothread_mutex_destroy(&mutex);
    foothread_exit();
    int root=-1;
    for(int i=0;i<numnodes;i++)
    {
        if(parent[i]==i)
        {
            root = i;
            break;
        }
    }
    printf("Sum calculated: %d\n",sum[root]);
}