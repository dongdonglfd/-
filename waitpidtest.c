#include<stdio.h>
#include<unistd.h>
#include<pthread.h>
#include<errno.h>
#include<stdlib.h>
#include<sys/types.h>
#include<sys/wait.h>
int main()
{
    int i=0;
    pid_t pid,wpid;
    for(i=0;i<5;i++)
    {
        pid=fork();
        if(pid==0)
        {
            break;
        }
    }
    if(5==i)
    {
        // while((wpid=wait(NULL))!=-1)//阻塞等待子进程结束，回收
        // {
        //     printf("wait child %d\n",wpid);
        // }
         while((wpid=waitpid(-1,NULL,0))!=-1)//使用waitpid阻塞等待子进程结束，回收
        {
            printf("wait child %d\n",wpid);
        }
    }
    else
    {
        sleep(i);
        printf("%dth child,pid=%d\n",i+1,getpid());
    }
    return 0;
}