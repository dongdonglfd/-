#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include<arpa/inet.h>
int main()
{
    
    //1.创建通信套接字
    int cfd=socket(AF_INET,SOCK_STREAM,0);
    if(cfd==-1)
    {
        perror("socket");
        exit(0);
    }
    //2.连接服务器
    struct sockaddr_in addr;
    addr.sin_family=AF_INET;//IPV4
    addr.sin_port=htons(8989);//网络字节序
    //变成大端
    inet_pton(AF_INET,"127.0.0.1/8",&addr.sin_addr.s_addr);
    int ret=connect(cfd,(struct sockaddr*)&addr,sizeof(addr));
    if(ret==-1)
    {
        perror("connect");
        exit(0);
    }
    int num=0;
    //3.通信
    while(1)
    {
        
        //发送数据
        char buf[1024];
        snprintf(buf,sizeof(buf),"hello world,%d.....",num++);
        send(cfd,buf,strlen(buf)+1,0);
        //接受数据
        memset(buf,0,sizeof(buf));
        ssize_t len=recv(cfd,buf,sizeof(buf),0);
        if(len==0)
        {
            printf("服务器已经断开了连接\n");
            break;
        }
        else if(len>0)
        {
            printf("recv buf:%s\n",buf);
        }
        else
        {
            perror("recv");
            break;
        }
        sleep(1);
        
    }
    //4.断开连接
        close(cfd);
        return 0;

}
