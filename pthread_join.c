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
   
    // return (void *)74;
    pthread_exit((void *)"hello");
}
int main()
{
    pthread_t tid;
    char *retval;
    int ret= pthread_create(&tid,NULL,tfn,NULL);
    if(ret!=0)
    {
        fprintf(stderr,"err:%s\n",strerror(ret));
    }
    //回收子线程退出值
    ret=pthread_join(tid,(void**)&retval);//阻塞回收线程
    if(ret!=0)
    {
        fprintf(stderr,"pthread_join err:%s\n",strerror(ret));
    }
    // printf("child thread exit with %ld\n",(long)retval);
    printf("child thread exit with %s\n",(char *)retval);
    pthread_exit((void*)0);
    
 
}