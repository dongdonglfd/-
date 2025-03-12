#include<stdio.h>
#include<unistd.h>
#include<pthread.h>
#include<errno.h>
#include<stdlib.h>
#include<sys/types.h>
#include<sys/wait.h>
#include<error.h>
#include<string.h>
void *tfn(void *arg)
{
    printf("tfn:pid=%d,pthread_id=%lu\n",getpid(),pthread_self());
    return NULL;
}
int main()
{
    pthread_t tid;
    //创建子线程
    int ret=pthread_create(&tid,NULL,tfn,NULL);
    if(ret!=0)
    {
        fprintf(stderr,"err:%s\n",strerror(ret));
    }
    //设置线程分离
    ret=pthread_detach(tid);
    if(ret!=0)
    {
        fprintf(stderr,"err:%s\n",strerror(ret));
    }
    sleep(1);
    ret=pthread_join(tid,NULL);
    if(ret!=0)
    {
        fprintf(stderr,"err:%s\n",strerror(ret));
        exit(1);
    
    }
    printf("main:pid=%d,pthread_id=%lu\n",getpid(),pthread_self());
    pthread_exit((void*)0);//退出主线程

}