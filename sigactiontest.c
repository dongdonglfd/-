#define _POSIX_C_SOURCE 200809L
#include<stdio.h>
#include<unistd.h>
#include<pthread.h>
#include<errno.h>
#include<stdlib.h>
#include<sys/types.h>
#include<sys/wait.h>
#include<signal.h>
void sig_catch(int signum)
{
    printf("catch %d\n",signum);
}
int main()
{
    struct sigaction act,oldact;
    act.sa_handler=sig_catch;
    sigemptyset(&(act.sa_mask));
    act.sa_flags=0;
    int ret=sigaction(SIGINT,&act,&oldact);
    if(ret==-1)
    {
        perror("sigaction error");
    }
    while(1);
    return 0;
}