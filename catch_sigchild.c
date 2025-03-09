#define _POSIX_C_SOURCE 200809L
#include<stdio.h>
#include<unistd.h>
#include<pthread.h>
#include<errno.h>
#include<stdlib.h>
#include<sys/types.h>
#include<sys/wait.h>
#include<signal.h>
void catch_child(int signum)
{
    pid_t wpid;
    while((wpid=wait(NULL))!=-1)
    {
        printf("catch child pid =%d\n",wpid); 
    }
}
int main()
{
    int i=0;
    pid_t pid;
    for(i=0;i<5;i++)
    {
        if((pid=fork())==0)
        {
            break;
        }
    }
    if(5==i)//父进程
    {
        struct sigaction act;
        act.sa_handler=catch_child;
        sigemptyset(&(act.sa_mask));
        act.sa_flags=0;
        sigaction(SIGCHLD,&act,NULL);
        printf("parent %d",getpid());
    }
    else
    {
        printf("child %d",getpid());
        return i;
    }
    return 0;
}