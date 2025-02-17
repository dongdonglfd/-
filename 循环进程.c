#include<stdio.h>
#include<unistd.h>
#include<pthread.h>
int main()
{
    int i=0;
    for(i=0;i<5;i++)
    {
        if(fork()==0)
        {
            break; //循环期间，子进程不参与循环
        }
    }
    if(5==i)
    {
        sleep(5);
        printf("parent\n");
    }
    else
    {
        sleep(i);
        printf("child %d\n",i+1);
    }
    return 0;
}