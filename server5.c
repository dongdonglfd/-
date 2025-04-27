#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include<arpa/inet.h>
#include<sys/epoll.h>
int main()
{
    //1.创建监听套接字
    int lfd=socket(AF_INET,SOCK_STREAM,0);//AF_INET：表示使用 IPv4 地址族。SOCK_STREAM：表示使用面向连接的字节流协议，即 TCP
    if(lfd==-1)
    {
        perror("socket");
        exit(0);
    }
    int optval=1;
    setsockopt(lfd,SOL_SOCKET,SO_REUSEADDR,&optval,sizeof(int));
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
    //4.创建epoll模型
    int epfd=epoll_create(100);
    if(epfd==-1)
    {
        perror("epoll_create");
        exit(0);
    }
    //5.将要检测的节点添加到epoll模型中
    struct epoll_event ev;
    ev.events=EPOLLIN;//检测lfd的读缓冲区
    ev.data.fd=lfd;//要检测的文件描述符
    int r=epoll_ctl(epfd,EPOLL_CTL_ADD,lfd,&ev);
    if(r==-1)
    {
        perror("epoll_ctl");
        exit(0);
    }
    struct epoll_event evs[1024];
    int size=sizeof(evs)/sizeof(evs[0]);
    //6.不停的委托内核检测epoll模型中文件描述符状态
    while(1)
    {
        int num=epoll_wait(epfd,evs,size,-1);
        printf("num=%d\n",num);
        //遍历evs数组，个数就是返回值
        for(int i=0;i<num;i++)
        {
            int curfd=evs[i].data.fd;
            if(curfd==lfd)
            {
                int cfd=accept(lfd,NULL,NULL);
                ev.events=EPOLLIN;
                ev.data.fd=cfd;
                epoll_ctl(epfd,EPOLL_CTL_ADD,cfd,&ev);
            }
            else
            {
                char buf[1024];
                memset(buf,0,sizeof(buf));
                ssize_t len=recv(curfd,buf,sizeof(buf),0);
                if(len==0)
                {
                    printf("客户端断开了连接\n");
                    epoll_ctl(epfd,EPOLL_CTL_DEL,curfd,NULL);
                    //先从树上删除再关闭
                    close(curfd);
                }
                else if(len>0)
                {
                    printf("recv date:%s\n",buf);
                    send(curfd,buf,sizeof(buf),0);
                }
                else
                {
                    perror("recv");
                    exit(0);
                }
            }
        }
    }
    close(lfd);
    return 0;

}