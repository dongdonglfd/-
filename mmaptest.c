#include<stdio.h>
#include<unistd.h>
#include<pthread.h>
#include<errno.h>
#include<stdlib.h>
#include<sys/types.h>
#include<sys/wait.h>
#include<fcntl.h>
#include<sys/mman.h>
#include<string.h>
int main()
{
    int fd=open("test.mmap",O_RDWR|O_CREAT|O_TRUNC,0644);
    ftruncate(fd,4);
    char*memp;
    memp=mmap(NULL,4,PROT_READ|PROT_WRITE,MAP_SHARED,fd,0);
    if(memp==MAP_FAILED)
    {
        perror("mmap error");
    }
    strcpy(memp,"xxx");
    printf("%s\n",memp);
    if(munmap(memp,4)==-1)
    {
        perror("munmap error");
    }
    close(fd);
}
//fd=open("文件名"，O_RDWR);
//mmap(NULL,有效文件大小,PROT_READ|PROT_WRITE,MAP_SHARED,fd,0);