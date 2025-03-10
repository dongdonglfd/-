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
    int i=(int)(long)arg;
    // int i=*((int*)arg);
    // sleep(i);
    if(i==2)
    {
        // exit(0);//退出当前进程
        // return NULL;//表示返回到调用者那里去
        pthread_exit(NULL);//退出当前线程
    }
    printf("%dth thread:pid=%d,tid=%lu\n",i+1,getpid(),pthread_self());
    return NULL;
}
int main()
{
    int i=0;
    pthread_t tid;
    for(i=0;i<5;i++)
    {
        int ret= pthread_create(&tid,NULL,tfn,(void *)(long)i);
        //int ret= pthread_create(&tid,NULL,tfn,(void *)&i);错误演示，i值会改变
        if(ret!=0)
        {
            fprintf(stderr,"err:%s\n",strerror(ret));
        }
    }
    sleep(i);
    printf("%d\n",i);
    printf("main:pid=%d,tid=%lu\n",getpid(),pthread_self());//获取线程id
 
}