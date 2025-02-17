#include<stdio.h>
#include<unistd.h>
#include<pthread.h>
int main()
{
    printf("===============\n");
    pid_t pid=fork();
    if(pid==0)//子进程
    {
        printf("child,child id=%d,parent id=%d\n",getpid(),getppid());
    }
    else if(pid>0)//父进程
    {
        printf("parent,child id=%d,parent id =%d,ppid=%d\n",pid,getpid(),getppid());
    }
    printf("=======\n");
    return 0;
}
//getpid 获取当前进程id
//getppid 获取父进程id# -
