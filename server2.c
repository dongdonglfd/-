#define _POSIX_SOURCE
#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include<arpa/inet.h>
#include<sys/wait.h>
#include<signal.h>
void working(int cfd);
void recycle()
{
    while(1)
    {
        pid_t pid=waitpid(-1,NULL,WNOHANG);
        if(pid>0)
        {
            printf("子进程被回收了，pid:%d",pid);
        }
        else
        {
            break;
        }
    }
}
int main()
{
    //1.创建监听的fd
    int lfd =socket(AF_INET,SOCK_STREAM,0);
    if(lfd==-1)
    {
        perror("socket");
        exit(0);
    }
    //2.绑定
    struct sockaddr_in addr;
    addr.sin_family=AF_INET;//IPV4
    addr.sin_port=htons(8989);//网络字节序
    addr.sin_addr.s_addr=INADDR_ANY;//0地址
    int ret=bind(lfd,(struct sockaddr*)&addr,sizeof(addr));
    if(ret==-1)
    {
        perror("bind");
        exit(0);
    }
    //3.设置监听
    ret=listen(lfd,128);
    if(ret==-1)
    {
        perror("listen");
        exit(0);
    }
    //注册信号捕捉
    struct sigaction act;
    act.sa_handler=recycle;
    act.sa_flags=0;
    sigemptyset(&act.sa_mask);
    sigaction(SIGCHLD,&act,NULL);
    //因为要接收多客户端的连接因此需要循环调用accept
    struct sockaddr_in cliaddr;
    socklen_t clilen=sizeof(cliaddr);
    while(1)
    {
        int cfd=accept(lfd,(struct sockaddr*)&cliaddr,&clilen);
        if(cfd==-1)
        {
            perror("accept");
            continue;
        }
        char myip[24];
        printf("客户端ip:%s,端口：%d\n",inet_ntop(AF_INET,&cliaddr.sin_addr.s_addr,myip,sizeof(myip)),ntohs(cliaddr.sin_port));
        //连接已经建立了，通信在子进程中处理
        pid_t pid=fork();
        if(pid==0)//子进程
        {
            //关闭监听的文件描述符
            close(lfd);
            working(cfd);
            exit(0);
        }
        else
        {
            //父进程关闭通信的文件描述符
            close(cfd);
        }
    }
    return 0;
}
void working(int cfd)
{
    while(1)
    {
        char buf[1024];
        memset(buf,0,sizeof(buf));
        //read阻塞函数，客户端不发数据还是就阻塞
        //客户端发送数据，阻塞解除
        ssize_t len=read(cfd,buf,sizeof(buf));
        if(len==0)
        {
            printf("客户端已经断开了连接\n");
            break;
        }
        else if(len==-1)
        {
            perror("read error");
            break;
        }
        else
        {
            printf("客户端say:%s\n",buf);
            //回复数据
            write(cfd,buf,sizeof(buf));
        }
    }
    close(cfd);
}