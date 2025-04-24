#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include<arpa/inet.h>
#include<sys/select.h>
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
    //4.初始化检测的集合
    fd_set reads,tmp;
    FD_ZERO(&reads);
    FD_SET(lfd,&reads);
    int nfds=lfd;
    //5.不停委托内核检测集合中文件描述符状态
    while(1)
    {
        tmp=reads;
        int num=select(nfds+1,&tmp,NULL,NULL,NULL);
        printf("num=%d\n",num);
        for(int i=0;i<=nfds;++i)
        {
            if(i==lfd&&FD_ISSET(lfd,&tmp))
            {
                int cfd=accept(lfd,NULL,NULL);
                FD_SET(cfd,&reads);
                nfds=nfds<cfd?cfd:nfds;
    
            }
            else
            {
               if(FD_ISSET(i,&tmp)) 
               {
                    char buf[1024];
                    memset(buf,0,sizeof(buf));
                    ssize_t len=recv(i,buf,sizeof(buf),0);
                    if(len==0)
                    {
                        printf("客户端断开了连接\n");
                        FD_CLR(i,&reads);
                        close(i);
                    }
                    else if(len>0)
                    {
                        printf("recv date:%s\n",buf);
                        send(i,buf,sizeof(buf),0);
                    }
                    else
                    {
                        perror("recv");
                        break;

                    }
               }

            }
        }

    }
    //6.断开连接
        close(lfd);
        return 0;

}
