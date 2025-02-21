#include<stdio.h>
#include<unistd.h>
#include<pthread.h>
#include<errno.h>
#include<stdlib.h>
#include<sys/types.h>
#include<sys/wait.h>
int main()
{
    int fd[2],i=0;
    pid_t pid;
    pipe(fd);
    for(i=0;i<2;i++)
    {
        if((pid=fork())==0)
        {
            break;
        }
    }
    if(i==0)//兄  ls
    {
        close(fd[0]);
        dup2(fd[1],STDOUT_FILENO);
        execlp("ls","ls",NULL);
    }
    else if(i==1)//弟  wc -l
    {
        close(fd[1]);
        dup2(fd[0],STDIN_FILENO);
        execlp("wc","wc","-l",NULL);
    }
    else//父
    {
        close(fd[0]);//父进程不参与管道的使用，实现管道单向流通
        close(fd[1]);
        for(i=0;i<2;i++)
        {
            wait(NULL);
        }
    }
    return 0;
}