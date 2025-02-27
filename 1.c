#define _POSIX_C_SOURCE 200809L  // 启用 POSIX 2008 标准
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <limits.h>

 
#define MAX_LINE 80  // 命令行的最大长度
 
// 解析输入字符串，返回参数数组和参数个数
char **parse_line(char *line, int *arg_count) {
    char **tokens = NULL;
    char *token, **tokens_backup;
    int buffer_size = 10, position = 0;
 
    if (line == NULL) return NULL;
 
    tokens = malloc(buffer_size * sizeof(char*));
    if (!tokens) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }
 
    token = strtok(line, " \t\r\n\a");
    while (token != NULL) {
        tokens[position] = token;
        position++;
 
        if (position >= buffer_size) {
            buffer_size += 10;
            tokens_backup = tokens;
            tokens = realloc(tokens, buffer_size * sizeof(char*));
            if (!tokens) {
                perror("realloc");
                free(tokens_backup);
                exit(EXIT_FAILURE);
            }
        }
 
        token = strtok(NULL, " \t\r\n\a");
    }
    tokens[position] = NULL;
    *arg_count = position;
    return tokens;
}
 
// 执行命令
void execute_command(char **args) {
    pid_t pid, wpid;
    int status;
 
    pid = fork();
    if (pid == 0) {  // 子进程
        if (execvp(args[0], args) == -1) {
            perror("shell");
        }
        exit(EXIT_FAILURE);  // 如果execvp失败，则退出子进程
    } else if (pid < 0) {  // fork失败
        perror("shell");
    } else {  // 父进程
        do {
            wpid = waitpid(pid, &status, WUNTRACED);
        } while (!WIFEXITED(status) && !WIFSIGNALED(status));//用于检查子进程是否已经正常退出或因信号而终止
    }
}
 
int main() {
    char *line = NULL;
    int arg_count;
    char **args;
    size_t bufsize = 0;  // getline()分配的缓冲区大小
    char *prompt = "osh> ";  // 提示符
 
    while (1) {
        printf("%s", prompt);
        fflush(stdout);//用于清空（刷新）标准输出缓冲区（stdout），确保文本被立即显示
 
        getline(&line, &bufsize, stdin);//用于存储读取的行
 
        if (strncmp(line, "exit", 4) == 0) {
            break;
        }
 
        args = parse_line(line, &arg_count);
        if (arg_count == 0) {
            continue;  // 如果输入为空，则继续循环
        }
 
        execute_command(args);
 
        free(args);  // 释放解析命令时分配的内存
    }
 
    free(line);  // 释放getline()分配的缓冲区
    return 0;
}