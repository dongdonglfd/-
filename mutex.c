#include<stdio.h>
#include<unistd.h>
#include<pthread.h>
#include<errno.h>
#include<stdlib.h>
#include<sys/types.h>
#include<sys/wait.h>
#include<error.h>
#include<string.h>
pthread_mutex_t mutex;
void *tfn(void *arg)
{
    srand(time(NULL));
    while(1)
    {
        pthread_mutex_lock(&mutex);
        printf("hello ");
        sleep(rand()%3);
        printf("world\n");
        pthread_mutex_unlock(&mutex);
        sleep(rand()%3);
    }
    return NULL;
}
int main()
{
    pthread_t tid;
    srand(time(NULL));
    pthread_mutex_init(&mutex,NULL);//初始化锁
    pthread_create(&tid,NULL,tfn,NULL);
    while(1)
    {
        pthread_mutex_lock(&mutex);//加锁
        printf("HELLO ");
        sleep(rand()%3);
        printf("WORLD\n");
        pthread_mutex_unlock(&mutex);//解锁，唤醒阻塞在锁上的子线程
        sleep(rand()%3); 
    }
    pthread_join(tid,NULL);
    return 0;

}