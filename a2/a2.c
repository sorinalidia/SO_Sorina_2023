#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <pthread.h>
#include "a2_helper.h"
#include <semaphore.h>
#include <fcntl.h>


#define NR_THREADS8 48
#define NR_THREADS5 6
#define NR_THREADS6 5
sem_t *semaphore1;
sem_t *semaphore2;
int c=0;
pthread_cond_t condition1=PTHREAD_COND_INITIALIZER;
pthread_mutex_t mutex1=PTHREAD_MUTEX_INITIALIZER;
void* thread_function(void* arg)
{
    int thread_num = *(int*)arg;
    pthread_mutex_lock(&mutex1);

    if(thread_num == 1)
    {
        info(BEGIN, 6, thread_num);
        c = 1;
        pthread_cond_signal(&condition1);
    }
    else if(thread_num == 4)
    {
        if(c == 1)
        {
            info(BEGIN, 6, thread_num);
        }
        else
        {
            pthread_cond_wait(&condition1, &mutex1);
            info(BEGIN, 6, thread_num);
        }
    }
    else if(thread_num == 3)
    {
        sem_wait(semaphore1);
        info(BEGIN, 6, thread_num);
    }
    else  
    {
        info(BEGIN, 6, thread_num);
    }

    if(thread_num == 1)
    {
        pthread_cond_wait(&condition1, &mutex1);
        info(END, 6, thread_num);
    }
    else if(thread_num == 4)
    {
        info(END, 6, thread_num);
        pthread_cond_signal(&condition1);
    }
    else if(thread_num == 3)
    {
        info(END, 6, thread_num);
        sem_post(semaphore2);
    }
    else 
    {
        info(END, 6, thread_num);
    }

    pthread_mutex_unlock(&mutex1);
    return NULL;
}

void* thread_function5(void* arg)
{
    int thread_num = *(int*)arg;
    // pthread_mutex_lock(&mutex1);
    if(thread_num == 2)
    {
        sem_wait(semaphore2);
        info(BEGIN, 5, thread_num);
    }
    else
    {
        info(BEGIN, 5, thread_num);
    }

    if(thread_num == 3)
    {
        info(END, 5, thread_num);
        sem_post(semaphore1);


    }
    else 
    {
        info(END, 5, thread_num);
    }
     //pthread_mutex_unlock(&mutex1);
    return NULL;
}


// void* thread_function(void* arg)
// {
//    // 
//     int thread_num = *(int*)arg;
//     pthread_mutex_lock(&mutex1);
//     //info(BEGIN, 6, thread_num);
//     if(thread_num==1)
//     {
//         info(BEGIN, 6, thread_num);
//         c=1;
//         pthread_cond_signal(&condition1);

//     }
//     else if(thread_num==4)
//     {
//         //pthread_cond_wait(&condition1,&mutex1);
//         if(c==1)
//         {
//             info(BEGIN, 6, thread_num);
    
//         }
//          else{
//             pthread_cond_wait(&condition1,&mutex1);
//             info(BEGIN, 6, thread_num);}

//     }
//     else if(thread_num==3)
//     {
//         sem_wait(semaphore1);
//         info(BEGIN,6,thread_num);
//     }
//     else  info(BEGIN, 6, thread_num);

//     if(thread_num==1){
//         pthread_cond_wait(&condition1,&mutex1);
//         info(END,6,thread_num);
//     }
//     else if(thread_num==4){
//         info(END,6,thread_num);
//         pthread_cond_signal(&condition1);
        
//     }else if(thread_num==3)
//     {
//         info(END,6,thread_num);
//         sem_post(semaphore2);
//     }
//     else info(END,6,thread_num);

// pthread_mutex_unlock(&mutex1);
//     return NULL;
// }
void* thread_function8(void* arg)
{
    int thread_num = *(int*)arg;
    info(BEGIN, 8, thread_num);

    info(END, 8, thread_num);
    return NULL;
}
// void* thread_function5(void* arg)
// {
//     int thread_num = *(int*)arg;
//     if(thread_num==2)
//     {
//           sem_wait(semaphore2);
//           info(BEGIN, 5, thread_num);
        
//     }else
//     info(BEGIN, 5, thread_num);
//     if(thread_num==3)
//     {
//     info(END, 5, thread_num);
//     sem_post(semaphore1);
//     }
//     else info(END, 5, thread_num);
   
//     return NULL;
// }

int main() {
    init();
    info(BEGIN,1,0);
    semaphore1=sem_open("/sem1",O_CREAT,0644,0);
    semaphore2=sem_open("/sem2",O_CREAT,0644,0);
    pthread_cond_init(&condition1,NULL);
    pthread_mutex_init(&mutex1,NULL);
    pid_t pid2, pid3, pid4, pid5, pid6, pid7, pid8;
   
    pid2 = fork();
    if (pid2 == 0) { // p2
        info(BEGIN,2,0);
        pid3 = fork();
        if (pid3 == 0) { // p3
            info(BEGIN,3,0);
            pid4 = fork();
            if (pid4 == 0) { // p4
                info(BEGIN,4,0);
                pid5 = fork();
                if (pid5 == 0) { // p5
                    info(BEGIN,5,0);
                      pthread_t threads[NR_THREADS5];
                    int thread_nums[NR_THREADS5];
                    for (int i = 0; i < NR_THREADS5; i++) {
                        thread_nums[i]=i+1;
                        pthread_create(&threads[i], NULL, thread_function5, &thread_nums[i]);
                    }
                    for (int i = 0; i < NR_THREADS5; i++) {
                        pthread_join(threads[i], NULL);
                    }
                    info(END,5,0);
                    return 0;
                }
             
                pid6 = fork();
                if (pid6 == 0) { // p6
                    info(BEGIN,6,0);
                    pthread_t threads[NR_THREADS6];
                    int thread_nums[NR_THREADS6] = {1, 2, 3, 4, 5};
                    for (int i = 0; i < NR_THREADS6; i++) {
                        thread_nums[i]=i+1;
                        pthread_create(&threads[i], NULL, thread_function, &thread_nums[i]);
                    }
                    for (int i = 0; i < NR_THREADS6; i++) {
                        pthread_join(threads[i], NULL);
                    }

                    info(END,6,0);
                    return 0;

                }
                
                pid7 = fork();
                if (pid7 == 0) { // p7
                    info(BEGIN,7,0);
                    pid8 = fork();
                    if (pid8 == 0) { // p8
                        info(BEGIN,8,0);
                        pthread_t threads[NR_THREADS8];
                    int thread_nums[NR_THREADS8];
                    for (int i = 0; i < NR_THREADS8; i++) {
                        thread_nums[i]=i+1;
                        pthread_create(&threads[i], NULL, thread_function8, &thread_nums[i]);
                    }
                    for (int i = 0; i < NR_THREADS8; i++) {
                        pthread_join(threads[i], NULL);
                    }
                        info(END,8,0);
                        return 0;
                    }
                    waitpid(pid8, NULL, 0);
                    info(END,7,0);
                    return 0;
                }   waitpid(pid5,NULL,0);
                waitpid(pid6,NULL,0);
                waitpid(pid7, NULL, 0);
                waitpid(pid4, NULL, 0);
                info(END,4,0);
                
                return 0;
            }
            waitpid(pid4, NULL, 0);
            info(END,3,0);
            return 0;
        }
        waitpid(pid3, NULL, 0);
        info(END,2,0);
        return 0;
    }
    waitpid(pid2, NULL, 0);
     sem_close(semaphore1);
    sem_unlink("/sem1");
     sem_close(semaphore2);
    sem_unlink("/sem2");
    info(END,1,0);
    return 0;
}