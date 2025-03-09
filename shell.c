#define _POSIX_C_SOURCE 200809L  // 启用POSIX特性
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <signal.h>
#include <errno.h>

#define MAX_ARGS 64    // 单个命令最大参数个数
#define MAX_CMDS 16    // 管道支持的最大命令数
#define MAX_INPUT 1024 // 输入缓冲区大小

// 命令结构体定义
typedef struct {
    char* args[MAX_ARGS]; // 命令参数数组
    char* input;          // 输入重定向文件
    char* output;         // 输出重定向文件
    int append;           // 输出是否追加模式
} Command;

// 管道结构体定义
typedef struct {
    Command cmds[MAX_CMDS]; // 命令数组
    int cmd_count;          // 命令数量
    int background;         // 是否后台运行
} Pipeline;

char prev_dir[PATH_MAX];    // 保存前一个工作目录
volatile sig_atomic_t child_pid = -1; // 前台子进程PID

/*---------- 信号处理 ----------*/
void handle_sigint(int sig) {
    // 只中断前台进程
    if (child_pid > 0) {
        kill(child_pid, SIGINT);
    }
}

void setup_signals() {
    struct sigaction sa;
    sa.sa_handler = handle_sigint;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    sigaction(SIGINT, &sa, NULL);
    
    // 忽略后台进程的SIGCHLD
    signal(SIGCHLD, SIG_IGN);
}

/*---------- 内建命令 ----------*/
int is_builtin(Command* cmd) {
    return strcmp(cmd->args[0], "cd") == 0 || 
           strcmp(cmd->args[0], "exit") == 0;
}

void handle_builtin(Command* cmd) {
    if (strcmp(cmd->args[0], "cd") == 0) {
        char* target = cmd->args[1];
        char cwd[PATH_MAX];
        
        if (!target) target = getenv("HOME");
        else if (strcmp(target, "-") == 0) target = prev_dir;
        
        if (getcwd(prev_dir, PATH_MAX) == NULL) {
            perror("getcwd");
            return;
        }
        
        if (chdir(target) {
            perror("cd");
        }
    } else if (strcmp(cmd->args[0], "exit") == 0) {
        exit(EXIT_SUCCESS);
    }
}

/*---------- 命令解析 ----------*/
void parse_command(char* token, Command* cmd) {
    int arg_idx = 0;
    memset(cmd, 0, sizeof(Command));
    
    while (token && arg_idx < MAX_ARGS-1) {
        if (strcmp(token, "<") == 0) {
            token = strtok(NULL, " \t\n");
            cmd->input = strdup(token);
        } else if (strcmp(token, ">") == 0) {
            token = strtok(NULL, " \t\n");
            cmd->output = strdup(token);
            cmd->append = 0;
        } else if (strcmp(token, ">>") == 0) {
            token = strtok(NULL, " \t\n");
            cmd->output = strdup(token);
            cmd->append = 1;
        } else {
            cmd->args[arg_idx++] = strdup(token);
        }
        token = strtok(NULL, " \t\n");
    }
    cmd->args[arg_idx] = NULL;
}

void parse_pipeline(char* input, Pipeline* pipeline) {
    char* cmd_str;
    char* saveptr;
    memset(pipeline, 0, sizeof(Pipeline));
    
    // 分割管道命令
    while ((cmd_str = strtok_r(input, "|", &saveptr)) && 
           pipeline->cmd_count < MAX_CMDS) {
        input = NULL;
        parse_command(strtok(cmd_str, " \t\n"), &pipeline->cmds[pipeline->cmd_count]);
        pipeline->cmd_count++;
    }
    
    // 检查后台运行标志
    if (pipeline->cmd_count > 0) {
        Command* last = &pipeline->cmds[pipeline->cmd_count-1];
        if (last->args[0] && strcmp(last->args[0], "&") == 0) {
            pipeline->background = 1;
            free(last->args[0]);
            last->args[0] = NULL;
        }
    }
}

/*---------- 命令执行 ----------*/
void execute_command(Command* cmd, int input_fd, int output_fd) {
    // 输入重定向
    if (cmd->input) {
        int fd = open(cmd->input, O_RDONLY);
        if (fd < 0) {
            perror("open input");
            exit(EXIT_FAILURE);
        }
        dup2(fd, STDIN_FILENO);
        close(fd);
    } else if (input_fd != STDIN_FILENO) {
        dup2(input_fd, STDIN_FILENO);
        close(input_fd);
    }

    // 输出重定向
    if (cmd->output) {
        int flags = O_WRONLY | O_CREAT;
        flags |= cmd->append ? O_APPEND : O_TRUNC;
        int fd = open(cmd->output, flags, 0644);
        if (fd < 0) {
            perror("open output");
            exit(EXIT_FAILURE);
        }
        dup2(fd, STDOUT_FILENO);
        close(fd);
    } else if (output_fd != STDOUT_FILENO) {
        dup2(output_fd, STDOUT_FILENO);
        close(output_fd);
    }

    execvp(cmd->args[0], cmd->args);
    perror("execvp");
    exit(EXIT_FAILURE);
}

void execute_pipeline(Pipeline* pipeline) {
    int fds[2];
    int prev_read = -1;
    pid_t pid;
    
    for (int i = 0; i < pipeline->cmd_count; i++) {
        if (i < pipeline->cmd_count-1) pipe(fds);
        
        if ((pid = fork()) == 0) {
            if (i > 0) {
                dup2(prev_read, STDIN_FILENO);
                close(prev_read);
            }
            if (i < pipeline->cmd_count-1) {
                close(fds[0]);
                execute_command(&pipeline->cmds[i], prev_read, fds[1]);
            } else {
                execute_command(&pipeline->cmds[i], prev_read, STDOUT_FILENO);
            }
        } else {
            if (i > 0) close(prev_read);
            if (i < pipeline->cmd_count-1) {
                prev_read = fds[0];
                close(fds[1]);
            }
        }
    }
    
    // 前台进程等待
    if (!pipeline->background) {
        child_pid = pid;
        while (waitpid(pid, NULL, 0) > 0);
        child_pid = -1;
    }
}

/*---------- 主循环 ----------*/
int main() {
    char input[MAX_INPUT];
    setup_signals();
    
    while (1) {
        // 显示提示符
        printf("\033[1;32mmy_shell:\033[0m ");
        fflush(stdout);
        
        // 读取输入
        if (!fgets(input, MAX_INPUT, stdin)) break;
        input[strcspn(input, "\n")] = 0;
        
        // 解析管道
        Pipeline pipeline;
        parse_pipeline(input, &pipeline);
        
        if (pipeline.cmd_count == 0) continue;
        
        // 处理内建命令
        if (pipeline.cmd_count == 1 && is_builtin(&pipeline.cmds[0])) {
            handle_builtin(&pipeline.cmds[0]);
            continue;
        }
        
        // 执行外部命令
        execute_pipeline(&pipeline);
    }
    
    return 0;
}