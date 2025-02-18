#include<stdio.h>
#include<unistd.h>
#include<pthread.h>
#include<errno.h>
#include<stdlib.h>
int main()
{                       
    pid_t pid =fork();
    if(pid==0)
    {
        char*argvv[]={"ls","-l","-F",NULL};
        // execlp("ls","ls","-l","-F",NULL); //在环境变量PATH中寻找
        // execl("/bin/ls","ls","-l","-F",NULL);//在指定路径寻找
        execv("/bin/ls",argvv);
        perror("/bin/ls exec error");
        exit(1);
    }
    else if(pid>0)
    {
        sleep(1);
        printf("parent\n");
    }
}