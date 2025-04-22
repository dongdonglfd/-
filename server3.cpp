#include<iostream>
#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include<arpa/inet.h>
#include<sys/wait.h>
#include<signal.h>
#include<thread>
using namespace std;
int fds[1024];
void *working(void *arg)
{
    int cfd=*(int *)arg;
    //通信
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
    *(int *)arg=-1;
    return NULL;
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
    struct sockaddr_in cliaddr;
    socklen_t clilen=sizeof(cliaddr);
    memset(fds,-1,sizeof(fds));
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
        int *ptr=NULL;
        int len=sizeof(fds)/sizeof(int);
        for(int i=0;i<len;i++)
        {
            if(fds[i]==-1)
            {
                fds[i]=cfd;
                ptr=&fds[i];
                break;
            }
        }
        //创建子线程
        thread t(working,ptr);
        t.detach();

    }
    close(lfd);
    return 0;
}
