#include<stdio.h>
#include<unistd.h>
#include<pthread.h>
#include<errno.h>
#include<stdlib.h>
#include<sys/types.h>
#include<sys/wait.h>
#include<sys/signal.h>
#include<signal.h>
// void print_pedset(sigset_t *set)
// {
//     int i=0;
//     for(i=0;i<32;i++)
//     {
//         if(sigismember(set,i))
//         {
//             putchar('1');
//         }
//         else{
//             putchar('0');
//         }
//     }
// }
// int main()
// {
//     sigset_t set,oldset,pedset;
//     sigemptyset(&set);
//     sigaddset(&set,SIGINT);
//     int ret =sigprocmask(SIG_BLOCK,&set,&oldset);
//     if(ret==-1)
//     {
//         perror("sigprocmask error");
//     }
//     sigpending(&pedset);

//     print_pedset(&pedset);

//     return 0;
// }
void sig_catch(int signum)
{
    printf("catch %d\n",signum);
}
int main()
{
    signal(SIGINT,sig_catch);
    while(1);

    return 0;
    
}
// void sig_catch(int signum) {
//     printf("Caught signal %d\n", signum);
// }
 
// int main() {
//     signal(SIGINT, sig_catch);
 
//     // 无限循环，等待信号
//     while (1) {
//         // 可以添加一些其他逻辑，比如 sleep(1); 来减少 CPU 使用率
//     }
 
//     // 注意：由于有无限循环，下面的 return 语句实际上永远不会被执行
//     return 0;
// }