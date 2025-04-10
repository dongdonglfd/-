#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include<arpa/inet.h>
int main()
{
    //1.创建监听套接字
    int lfd=socket(AF_INET,SOCK_STREAM,0);
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
    //4.等待并接受客户端连接
    struct sockaddr_in cliaddr;
    socklen_t clilen=sizeof(cliaddr);
    int cfd=accept(lfd,(struct sockaddr*)&cliaddr,&clilen);
    if(cfd==-1)
    {
        perror("accept");
        exit(0);
    }
    //5.通信
    while(1)
    {
        //接受数据
        char buf[1024];
        memset(buf,0,sizeof(buf));
        ssize_t len=recv(cfd,buf,sizeof(buf),0);
        if(len==0)
        {
            printf("客户端已经断开了连接\n");
            break;
        }
        else if(len>0)
        {
            printf("recv buf:%s\n",buf);
            //回复数据
            send(cfd,buf,strlen(buf)+1,0);
        }
        else
        {
            perror("recv");
            break;
        }
        
    }
    //6.断开连接
        close(cfd);
        close(lfd);
        return 0;

}
