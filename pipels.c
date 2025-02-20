#include<stdio.h>
#include<unistd.h>
#include<pthread.h>
#include<errno.h>
#include<stdlib.h>
#include<sys/types.h>
#include<sys/wait.h>
int main()
{
    pid_t pid;
    int fd[2];
    pipe(fd);
    pid=fork();//ls | wc -l
    if(pid==0)//子进程实现wc -l
    {   
        close(fd[1]);//子进程读管道，关闭写端
        dup2(fd[0],STDIN_FILENO);//这一行将管道的写端（fd[1]）复制到标准输出（STDOUT_FILENO，通常是文件描述符1）
        execlp("wc","wc","-l",NULL);
    }
    else if(pid>0)
    {   
        close(fd[0]);//父进程写管道，关闭读管道
        dup2(fd[1],STDOUT_FILENO);//将写入屏幕ls的结果，写入到管道写端
        execlp("ls","ls","-l",NULL);
       
    }
    return 0;
}