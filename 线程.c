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
    printf("tfn:%lu\n",pthread_self());//获取线程id
    printf("%d\n",getpid());
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
    printf("main:%lu\n",pthread_self());//获取线程id
    printf("%d\n",getpid());
    sleep(1);//给子线程执行时间
    return 0;//释放进程地址空间
}