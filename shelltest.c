#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <fcntl.h>
#include <signal.h>
#include <dirent.h>

#define MAX_ARGS 64
#define MAX_CMDS 16
#define MAX_INPUT 1024

typedef struct {
    char *args[MAX_ARGS];   // 命令参数
    char *input;            // 输入重定向文件
    char *output;           // 输出重定向文件
    int append;             // 是否为追加模式
} Command;

typedef struct {
    Command cmds[MAX_CMDS]; // 命令数组
    int cmd_count;          // 命令数量
    int background;         // 是否后台运行
} Pipeline;

char prev_dir[1024] = {0};  // 存储上一个工作目录

// 打印彩色提示符
void print_prompt() {
    char current_dir[1024];
    getcwd(current_dir, sizeof(current_dir));
    printf("\033[1;32mxxx-super-shell:\033[0m \033[1;34m%s\033[0m$ ", current_dir);
    fflush(stdout);  // 确保立即输出
}

// 判断是否为内建命令
int is_builtin(Command *cmd) {
    return cmd->args[0] && strcmp(cmd->args[0], "cd") == 0;
}

// 处理内建命令
void handle_builtin(Command *cmd) {
    if (strcmp(cmd->args[0], "cd") == 0) {
        char *target = cmd->args[1];
        char current[1024];

        getcwd(current, sizeof(current));
        if (!target) {
            target = getenv("HOME");
        } else if (strcmp(target, "-") == 0) {
            target = prev_dir[0] ? prev_dir : current;
        }

        if (chdir(target) == 0) {
            strncpy(prev_dir, current, sizeof(prev_dir));
        } else {
            perror("cd");
        }
    }
}

// 解析输入到管道结构体
void parse_pipeline(char *input, Pipeline *pipeline) {
    char *token;
    Command *current = &pipeline->cmds[0];
    pipeline->cmd_count = 0;
    pipeline->background = 0;

    // 初始化第一个命令
    memset(current, 0, sizeof(Command));

    // 预处理：去除换行符
    input[strcspn(input, "\n")] = 0;

   while ((token = strtok(input, " \t\n")) != NULL) {
        input = NULL;

        if (strcmp(token, "|") == 0) {
            if (++pipeline->cmd_count >= MAX_CMDS) break;
            current = &pipeline->cmds[pipeline->cmd_count];
            memset(current, 0, sizeof(Command));
        } else if (strcmp(token, "<") == 0) {
            current->input = strdup(strtok(NULL, " \t"));
        } else if (strcmp(token, ">") == 0) {
            current->output = strdup(strtok(NULL, " \t"));
            current->append = 0;
        } else if (strcmp(token, ">>") == 0) {
            current->output = strdup(strtok(NULL, " \t"));
            current->append = 1;
        } else if (strcmp(token, "&") == 0) {
            pipeline->background = 1;
        } else {
            int i = 0;
            while (current->args[i] && i < MAX_ARGS-1) i++;
            if (i < MAX_ARGS-1) {
                current->args[i] = strdup(token);
            }
        }
    }
    pipeline->cmd_count++;
}

// 清理管道内存
void cleanup_pipeline(Pipeline *pipeline) {
    for (int i = 0; i < pipeline->cmd_count; i++) {
        for (int j = 0; j < MAX_ARGS && pipeline->cmds[i].args[j]; j++) {
            free(pipeline->cmds[i].args[j]);
        }
        free(pipeline->cmds[i].input);
        free(pipeline->cmds[i].output);
    }
}

// 执行管道命令
void execute_pipeline(Pipeline *pipeline) {
    int fds[2];
    int prev_read = -1;
    pid_t pids[MAX_CMDS];
    int pid_index = 0;

    for (int i = 0; i < pipeline->cmd_count; i++) {
        Command *cmd = &pipeline->cmds[i];
        
        if (i < pipeline->cmd_count - 1) {
            if (pipe(fds) < 0) {
                perror("pipe");
                return;
            }
        }

        pid_t pid = fork();
        if (pid < 0) {
            perror("fork");
            return;
        } else if (pid == 0) { // 子进程
            signal(SIGINT, SIG_DFL);

            // 输入处理
            if (cmd->input) {
                int fd = open(cmd->input, O_RDONLY);
                if (fd < 0) {
                    perror("open input");
                    exit(EXIT_FAILURE);
                }
                dup2(fd, STDIN_FILENO);
                close(fd);
            } else if (prev_read != -1) {
                dup2(prev_read, STDIN_FILENO);
                close(prev_read);
            }

            // 输出处理
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
            } else if (i < pipeline->cmd_count - 1) {
                dup2(fds[1], STDOUT_FILENO);
                close(fds[1]);
            }

            close(fds[0]);
            execvp(cmd->args[0], cmd->args);
            perror("execvp");
            exit(EXIT_FAILURE);
        } else { // 父进程
            pids[pid_index++] = pid;
            if (prev_read != -1) close(prev_read);
            if (i < pipeline->cmd_count - 1) {
                prev_read = fds[0];
                close(fds[1]);
            }
        }
    }

    // 等待前台进程
    if (!pipeline->background) {
        for (int i = 0; i < pid_index; i++) {
            waitpid(pids[i], NULL, 0);
        }
    }
}

int main() {
    signal(SIGINT, SIG_IGN);
    signal(SIGCHLD, SIG_IGN);

    char input[MAX_INPUT];
    while (1) {
        print_prompt();
        
        if (fgets(input, MAX_INPUT, stdin) == NULL) {
            printf("\n");
            break; // 处理Ctrl+D
        }

        Pipeline pipeline = {0};
        parse_pipeline(input, &pipeline);

        if (pipeline.cmd_count == 1 && is_builtin(&pipeline.cmds[0])) {
            handle_builtin(&pipeline.cmds[0]);
        } else if (pipeline.cmd_count > 0) {
            execute_pipeline(&pipeline);
        }

        cleanup_pipeline(&pipeline);
    }
    
    return 0;
}