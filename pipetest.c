#include<stdio.h>
#include<unistd.h>
#include<pthread.h>
#include<errno.h>
#include<stdlib.h>
#include<sys/types.h>
#include<sys/wait.h>
int main()
{
    int fd[2];
    int ret=pipe(fd);
    if(ret==-1)
    {
        perror("pipe error");
    }
    pid_t pid=fork();
    if(pid==0)
    {
        close(fd[1]);//子进程读管道，关闭写端
        char buf[1024]={0};
        read(fd[0],buf,sizeof(buf));
        printf("read:%s",buf);
        close(fd[0]);
    }
    else
    {
        close(fd[0]);//父进程写管道，关闭读端
        write(fd[1],"hello\n",6);
        wait(NULL);
        close(fd[1]);
    }
    return 0;
}