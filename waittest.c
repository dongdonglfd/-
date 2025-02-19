#include<stdio.h>
#include<unistd.h>
#include<pthread.h>
#include<errno.h>
#include<stdlib.h>
#include<sys/types.h>
#include<sys/wait.h>
int main()
{
    int status=0;
    pid_t pid=fork();
    if(pid==-1)
    {
        perror("fork err");
    }
    else if(pid==0)
    {
        printf("child,pid=%d\n",getpid());
        sleep(3);
        exit(10);
    }
    else
    {
        pid_t wpid=wait(&status);//保持子进程退出的状态
        if(wpid==-1)
        {
            perror("fork err");
        }
        if(WIFEXITED(status))//宏函数为真，说明子进程正常终止
        {
            //获取退出码
            printf("parent,pid=%d child,exit code=%d\n",wpid,WEXITSTATUS(status));
        }
        else if(WIFSIGNALED(status))//宏函数为真，说明子进程被信号终止
        {
            //获取信号编号
            printf("parent,pid=%d child,killednby %d signal\n",wpid,WTERMSIG(status));
        }

    }
}