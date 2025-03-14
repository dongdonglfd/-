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
struct msg
{
    struct msg *next;
    int num;
};
struct msg *head;
//静态初始化一个条件变量和一个互斥量
pthread_cond_t has_product =PTHREAD_COND_INITIALIZER;
pthread_mutex_t lock=PTHREAD_MUTEX_INITIALIZER;
void *consumer()
{
    struct msg *mp;
    for(;;)
    {   //加锁互斥量
        pthread_mutex_lock(&lock);
        while(head==NULL)
        {
            pthread_cond_wait(&has_product,&lock);//阻塞
        }
        mp=head;
        head=mp->next;
        pthread_mutex_unlock(&lock);
        printf("consumer%lu---%d\n",pthread_self(),mp->num);
        free(mp);
        sleep(rand()%3);



    }
}
void *producer()
{
    struct msg *mp;
    for(;;)
    {
        mp=malloc(sizeof(struct msg));
        mp->num=rand()%1000+1;//生产数据
        printf("------producer------%d\n",mp->num);
        pthread_mutex_lock(&lock);
        mp->next=head;
        head=mp;
        pthread_mutex_unlock(&lock);
        pthread_cond_signal(&has_product);
        sleep(rand()%3);

    }
}
int main()
{
    pthread_t pid,cid;
    srand(time(NULL));
    pthread_create(&pid,NULL,producer,NULL);
    pthread_create(&cid,NULL,consumer,NULL);
    pthread_join(pid,NULL);
    pthread_join(cid,NULL);


}