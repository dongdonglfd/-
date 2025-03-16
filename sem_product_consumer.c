#include<stdio.h>
#include<unistd.h>
#include<pthread.h>
#include<errno.h>
#include<stdlib.h>
#include<sys/types.h>
#include<sys/wait.h>
#include<error.h>
#include<string.h>
#include<time.h>
#include<semaphore.h>
#define NUM 5
int queue[NUM];
sem_t blank_number,product_number;//空格子信号量，产品信号量
void *producer(void *arg)
{
    int i=0;
    while(1)
    {
        sem_wait(&blank_number);//阻塞等待
        queue[i]=rand()%1000+1;
        printf("------producer-----%d\n",queue[i]);
        sem_post(&product_number);//产品数+1
        i=(i+1)%NUM;//借助下标实现循环
        sleep(rand()%1);

    }
}
void *consumer(void*arg)
{
    int i=0;
    while(1)
    {
        sem_wait(&product_number);//消费产品--，为0,则阻塞等待
        printf("-consumer-%d/n",queue[i]);
        queue[i]=0;//消费一个产品
        sem_post(&blank_number);
        i=(i+1)%NUM;
        sleep(rand()%3);
    }
}
int main()
{
    pthread_t pid,cid;
    sem_init(&blank_number,0,NUM);//初始化空格子数为5
    sem_init(&product_number,0,0);//产品数为0
    pthread_create(&pid,NULL,producer,NULL);
    pthread_create(&cid,NULL,consumer,NULL);
    pthread_join(pid,NULL);
    pthread_join(cid,NULL);
    sem_destroy(&blank_number);
    sem_destroy(&product_number);

}