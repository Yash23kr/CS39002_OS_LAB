
#include <stdio.h>
#include <stdlib.h>

#include <string.h>   /* Needed for strcpy() */
#include <time.h>     /* Needed for time() to seed the RNG */
#include <unistd.h>   /* Needed for sleep() and usleep() */
#include <pthread.h>  /* Needed for all pthread library calls */

#include "event.h"

pthread_cond_t doccond=PTHREAD_COND_INITIALIZER;
pthread_mutex_t docmutex=PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t repcond=PTHREAD_COND_INITIALIZER;
pthread_mutex_t repmutex=PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t reportercond=PTHREAD_COND_INITIALIZER;
pthread_mutex_t reportermutex=PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t patientcond=PTHREAD_COND_INITIALIZER;
pthread_mutex_t patientmutex=PTHREAD_MUTEX_INITIALIZER;
pthread_barrier_t barrier;
int docdone=0;
int doctime=0;
int patient_turn=0;
int rep_turn=0;
int reporter_turn=0;
int curtime=0;
int patient_num=0;
    int rep_num=0;
    int reporter_count=0;
    int waiting_patients=0;
    int waiting_reps=0;
    int waiting_reporters=0;
int processed_patients=0;
int processed_reps=0;
int processed_reporters=0;


void* doctor(void *attr);
void* patient(void *attr);
void* salesrep(void *attr);
void* reporter(void *attr);
int patient_duration[26];
int salesrep_duration[5];
int reporter_duration[100];
char * inttotime(int curtime)
{
    int curhour=9;
    while(curtime<0)
    {
        curtime+=60;
        curhour--;
    }
    while(curtime>=60)
    {
        curhour++;
        curtime-=60;
    }
    char *time = (char *)malloc(10*sizeof(char));
    if(curhour<12)
    {
        sprintf(time,"%02d:%02d AM",curhour,curtime);
    }
    else if(curhour==12)
    {
        sprintf(time,"%02d:%02d PM",curhour,curtime);
    }
    else
    {
        sprintf(time,"%02d:%02d PM",curhour-12,curtime);
    }
    return time;
}
int main()
{
    eventQ E;
    int i;
    event e;
    
    pthread_attr_t attr;
    pthread_t tid;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
    pthread_barrier_init(&barrier, NULL, 3);

    E = initEQ("arrival.txt");
    
    pthread_create(&tid, &attr, doctor,NULL);

    
    while(!emptyQ(E))
    {
        e = nextevent(E);
        while(!emptyQ(E)&&nextevent(E).time<=curtime)
        {
            if(e.type=='R')
            {
                printf("\t\t[%s] Reporter %d arrives\n",inttotime(e.time),1+reporter_count++);
                reporter_duration[reporter_count]=e.duration;
                pthread_create(&tid, &attr, reporter, (void *)reporter_count);
                waiting_reporters++;
            }
            if(e.type=='P')
            {
                if(patient_num>=25)
                {
                    printf("\t\t[%s] Patient %d arrives\n",inttotime(e.time),1+patient_num++);
                    printf("\t\t[%s] Patient %d leaves (Queue exhausted)\n",inttotime(e.time),patient_num);
                    E = delevent(E);
                    continue;
                }
                printf("\t\t[%s] Patient %d arrives\n",inttotime(e.time),1+patient_num++);
                patient_duration[patient_num]=e.duration;
                pthread_create(&tid, &attr, patient, (void *)patient_num);
                waiting_patients++;
            }
            if(e.type=='S')
            {
                if(rep_num>=3)
                {
                    printf("\t\t[%s] Sales representative %d arrives\n",inttotime(e.time),1+rep_num++);
                    printf("\t\t[%s] Sales representative %d leaves (Queue exhausted)\n",inttotime(e.time),rep_num);
                    E = delevent(E);
                    continue;
                }
                printf("\t\t[%s] Sales representative %d arrives\n",inttotime(e.time),1+rep_num++);
                salesrep_duration[rep_num]=e.duration;
                pthread_create(&tid, &attr, salesrep, (void *)rep_num);
                waiting_reps++;
            }
            E = delevent(E);
            e = nextevent(E);
            //printf("\t\tNext event temp1: %c %d %d\n",e.type,e.time,e.duration);
        }
        if(e.time>curtime)break;
        if(e.type=='E')
        {
            E = delevent(E);
            e = nextevent(E);
            //printf("\t\tNext event: %c %d %d\n",e.type,e.time,e.duration);
        }
    }
    printf("\t\t[%s] Doctor arrives\n",inttotime(0));
    while(1)
    {
        
        int done=0;
        e = nextevent(E);
        while(!emptyQ(E)&&nextevent(E).time<=curtime)
        {
            e = nextevent(E);
            if(e.type=='R')
            {
                printf("\t\t[%s] Reporter %d arrives\n",inttotime(e.time),1+reporter_count++);
                reporter_duration[reporter_count]=e.duration;
                pthread_create(&tid, &attr, reporter, (void *)reporter_count);
                waiting_reporters++;
            }
            if(e.type=='P')
            {
                if(patient_num>=25)
                {
                    printf("\t\t[%s] Patient %d arrives\n",inttotime(e.time),1+patient_num++);
                    printf("\t\t[%s] Patient %d leaves (Queue exhausted)\n",inttotime(e.time),patient_num);
                    E = delevent(E);
                    continue;
                }
                printf("\t\t[%s] Patient %d arrives\n",inttotime(e.time),1+patient_num++);
                patient_duration[patient_num]=e.duration;
                pthread_create(&tid, &attr, patient, (void*)patient_num);
                waiting_patients++;
            }
            if(e.type=='S')
            {
                if(rep_num>=3)
                {
                    printf("\t\t[%s] Sales rep %d arrives\n",inttotime(e.time),1+rep_num++);
                    printf("\t\t[%s] Sales rep %d leaves (Queue exhausted)\n",inttotime(e.time),rep_num);
                    E = delevent(E);
                    continue;
                }
                printf("\t\t[%s] Sales rep %d arrives\n",inttotime(e.time),1+rep_num++);
                salesrep_duration[rep_num]=e.duration;
                pthread_create(&tid, &attr, salesrep, (void*)rep_num);
                waiting_reps++;
            }
            E = delevent(E);
            e = nextevent(E);
            //printf("\t\tNext event temp1: %c %d %d\n",e.type,e.time,e.duration);
        }
        if(waiting_patients==0 && waiting_reporters==0 && waiting_reps==0&&patient_num>=25&&rep_num>=3)
        {
            pthread_mutex_lock(&docmutex);
            docdone=1;
            pthread_cond_signal(&doccond);
            pthread_mutex_unlock(&docmutex);
            pthread_mutex_lock(&docmutex);
            while(docdone==1)
            {
                pthread_cond_wait(&doccond, &docmutex);
            }
            pthread_mutex_unlock(&docmutex);
            //printf("\t\tMain thread exiting\n");
            break;
        }
        int arr[]={0,0,0};
        if(waiting_reporters>0)
        {
            pthread_mutex_lock(&docmutex);
            docdone=1;
            pthread_cond_signal(&doccond);
            pthread_mutex_unlock(&docmutex);
            done=1;
            pthread_mutex_lock(&docmutex);
            while(docdone==1)
            {
                pthread_cond_wait(&doccond, &docmutex);
            }
            pthread_mutex_unlock(&docmutex);
            pthread_mutex_lock(&reportermutex);
            reporter_turn++;
            //waiting_reporters--;
            arr[0]++;
            pthread_cond_broadcast(&reportercond);
            pthread_mutex_unlock(&reportermutex);
            event temp;
            temp.duration = 0;
            temp.time = curtime+reporter_duration[reporter_turn];
            temp.type = 'D';
            E = addevent(E,temp);
        }
        else if(waiting_patients>0)
        {
            pthread_mutex_lock(&docmutex);
            docdone=1;
            pthread_cond_signal(&doccond);
            pthread_mutex_unlock(&docmutex);
            pthread_mutex_lock(&docmutex);
            while(docdone==1)
            {
                pthread_cond_wait(&doccond, &docmutex);
            }
            pthread_mutex_unlock(&docmutex);
            done=1;
            pthread_mutex_lock(&patientmutex);
            patient_turn++;
            //waiting_patients--;
            arr[1]++;
            pthread_cond_broadcast(&patientcond);
            pthread_mutex_unlock(&patientmutex);
            event temp;
            temp.duration = 0;
            temp.time = curtime+patient_duration[patient_turn];
            temp.type = 'D';
            E = addevent(E,temp);

        }
        else if(waiting_reps>0)
        {
            pthread_mutex_lock(&docmutex);
            docdone=1;
            pthread_cond_signal(&doccond);
            pthread_mutex_unlock(&docmutex);
            done=1;
            pthread_mutex_lock(&docmutex);
            while(docdone==1)
            {
                pthread_cond_wait(&doccond, &docmutex);
            }
            pthread_mutex_unlock(&docmutex);
            pthread_mutex_lock(&repmutex);
            rep_turn++;
            //waiting_reps--;
            arr[2]++;
            pthread_cond_broadcast(&repcond);
            pthread_mutex_unlock(&repmutex);
            event temp;
            temp.duration = 0;
            temp.time = curtime+salesrep_duration[rep_turn];
            temp.type = 'D';
            E = addevent(E,temp);
        }
        if(done==1)
        {
            pthread_barrier_wait(&barrier);
            waiting_patients-=arr[1];
            waiting_reporters-=arr[0];
            waiting_reps-=arr[2];
            //printf("\t\t%d %d %d %d %d\n",waiting_patients,waiting_reporters,waiting_reps,patient_num,rep_num);
        }
        else{
            e = nextevent(E);
            curtime = e.time;
            //printf("\t\tChanging time manually\n");
        }
    }
    while(!emptyQ(E))
    {
        e = nextevent(E);
        if(e.type=='R')
        {
            printf("\t\t[%s] Reporter %d arrives\n",inttotime(e.time),1+reporter_count++);
            printf("\t\t[%s] Reporter %d leaves(session over)\n",inttotime(e.time),reporter_count);
        }
        if(e.type=='P')
        {
            printf("\t\t[%s] Patient %d arrives\n",inttotime(e.time),1+patient_num++);
            printf("\t\t[%s] Patient %d leaves(session over)\n",inttotime(e.time),patient_num);
        }
        if(e.type=='S')
        {
            printf("\t\t[%s] Sales rep %d arrives\n",inttotime(e.time),1+rep_num++);
            printf("\t\t[%s] Sales rep %d leaves(session over)\n",inttotime(e.time),rep_num);
        }
        E = delevent(E);
    }
    pthread_exit(NULL);
}


void* doctor(void *attr)
{
    while(1)
    {
        pthread_mutex_lock(&docmutex);
        while(docdone==0)
        {
            pthread_cond_wait(&doccond, &docmutex);
        }
        
        if(waiting_patients==0&&waiting_reporters==0&&waiting_reps==0&&patient_num>=25&&rep_num>=3)
        {
           
            printf("[%s] Doctor leaves\n",inttotime(curtime));
            docdone=0;
            pthread_cond_signal(&doccond);
            pthread_mutex_unlock(&docmutex);
            pthread_exit(NULL);
            
            break;
        }
        printf("[%s] Doctor receives visitor\n",inttotime(curtime));
        docdone=0;
        pthread_mutex_unlock(&docmutex);
        pthread_mutex_lock(&docmutex);
        pthread_cond_signal(&doccond);
        pthread_mutex_unlock(&docmutex);
        pthread_barrier_wait(&barrier);
    }
}

void* patient(void *attr)
{
    pthread_mutex_lock(&patientmutex);
    int patient_num = (int)attr;
    int duration = patient_duration[patient_num];
    while(patient_turn!=patient_num)
    {
        pthread_cond_wait(&patientcond, &patientmutex);
    }
    printf("\t[%s-%s] Patient %d is in the doctor's cabin\n",inttotime(curtime),inttotime(curtime+duration),patient_num);

    pthread_mutex_unlock(&patientmutex);
    curtime+=duration;
    pthread_barrier_wait(&barrier);
    pthread_exit(NULL);
}

void* salesrep(void *attr)
{
    pthread_mutex_lock(&repmutex);
    int rep_num = (int)attr;
    int duration = salesrep_duration[rep_num];
    while(rep_turn!=rep_num)
    {
        pthread_cond_wait(&repcond, &repmutex);
    }
    //rep_done=0;
    
    printf("\t[%s-%s] Sales rep %d is in the doctor's cabin\n",inttotime(curtime),inttotime(curtime+duration),rep_num);
    pthread_mutex_unlock(&repmutex);
    curtime+=duration;
    pthread_barrier_wait(&barrier);
    pthread_exit(NULL);
}

void* reporter(void *attr)
{
    pthread_mutex_lock(&reportermutex);
    int reporter_count = (int)attr;
    int duration = reporter_duration[reporter_count];
    while(reporter_turn!=reporter_count)
    {
        pthread_cond_wait(&reportercond, &reportermutex);
    }
    //reporter_done=0;
    
    printf("\t[%s-%s] Reporter %d is in the doctor's cabin\n",inttotime(curtime),inttotime(curtime+duration),reporter_count);
    pthread_mutex_unlock(&reportermutex);
    curtime+=duration;
    pthread_barrier_wait(&barrier);
    pthread_exit(NULL);
}