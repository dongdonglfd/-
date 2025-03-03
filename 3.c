#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <limits.h>
#include <errno.h>

#define MAX_LINE 1024
#define MAX_ARGS 64
#define MAX_CMDS 10

typedef struct {
    char* args[MAX_ARGS];//命令参数
    char* input;// 输入重定向文件
    char* output;// 输出重定向文件
    int append;// 输出是否追加模式（1 表示 >>，0 表示 >）
} Command;

typedef struct {
    Command cmds[MAX_CMDS];// 管道中的多个命令
    int cmd_count;// 命令数量
    int background;// 是否后台运行（1 表示 &，0 表示前台）
} Pipeline;

static char prev_dir[PATH_MAX] = {0};
char *s=NULL;
// 函数声明
Pipeline parse_input(char* line);
void execute_pipeline(Pipeline* pipeline);
void handle_cd(char **args);

void sig_catch(int signum)
{
    
}
// 解析输入并构建Pipeline结构
Pipeline parse_input(char* line) {
    Pipeline pipeline = {0};
    char *saveptr1, *saveptr2;//保存 strtok_r 的内部状态的变量
    char *cmd_str, *token;// 临时存储分割后的字符串
    // int bg_flag = 0;// 后台运行标志

    // 处理换行符和后台运行标志
    line[strcspn(line, "\n")] = '\0';//用于去除字符串line末尾的换行符
    if (strlen(line) > 0 && line[strlen(line)-1] == '&') {
        pipeline.background = 1;// 标记为后台运行
        line[strlen(line)-1] = '\0';// 移除末尾的 &
    }

    // 分割管道命令
    cmd_str = strtok_r(line, "|", &saveptr1);
    while (cmd_str && pipeline.cmd_count < MAX_CMDS) {
        Command cmd = {0};
        int arg_count = 0;
        // char *redir_token = NULL;

        // 分割命令参数
        token = strtok_r(cmd_str, " \t", &saveptr2);
        while (token && arg_count < MAX_ARGS-1) {
            // 处理重定向符号
            if (strcmp(token, "<") == 0) {//处理输入重定向 <
                cmd.input = strtok_r(NULL, " \t", &saveptr2);// 获取下一个token作为文件名，Shell中的命令参数通常以空格或制表符分隔
                if (!cmd.input) {
                    fprintf(stderr, "syntax error near unexpected token '<'\n");
                    break;
                }
            } else if (strcmp(token, ">") == 0 || strcmp(token, ">>") == 0) {//处理输出重定向 > 或 >> 
                cmd.append = (strcmp(token, ">>") == 0);
                cmd.output = strtok_r(NULL, " \t", &saveptr2);// 获取输出文件名
                if (!cmd.output) {
                    fprintf(stderr, "syntax error near unexpected token '%s'\n", token);
                    break;
                }
            } else {
                cmd.args[arg_count++] = token;//存储命令及参数
            }
            token = strtok_r(NULL, " \t", &saveptr2);
        }
        cmd.args[arg_count] = NULL;// 参数数组以NULL结尾

        if (arg_count > 0) {
            pipeline.cmds[pipeline.cmd_count++] = cmd;
        }
        cmd_str = strtok_r(NULL, "|", &saveptr1);// 获取下一个管道命令
    }
    return pipeline;
}

// 执行单个命令（带重定向）
void execute_command(Command* cmd, int input_fd, int output_fd) {
    // 输入重定向
    if (cmd->input) {
        int fd = open(cmd->input, O_RDONLY);
        if (fd == -1) {
            perror("open");
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
        if (fd == -1) {
            perror("open");
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

// 执行整个管道
void execute_pipeline(Pipeline* pipeline) {
    int prev_pipe_read = -1;
    pid_t pids[MAX_CMDS];
    int pid_count = 0;

    for (int i = 0; i < pipeline->cmd_count; i++) {
        int pipefd[2];
        if (i < pipeline->cmd_count - 1) {
            if (pipe(pipefd) == -1) {
                perror("pipe");
                exit(EXIT_FAILURE);
            }
        }

        pid_t pid = fork();
        if (pid == 0) { // 子进程
            // 处理输入重定向
            if (i > 0) {
                dup2(prev_pipe_read, STDIN_FILENO);
                close(prev_pipe_read);
            }

            // 处理输出重定向
            if (i < pipeline->cmd_count - 1) {
                close(pipefd[0]);  // 关闭不需要的读端
                dup2(pipefd[1], STDOUT_FILENO);
                close(pipefd[1]);
            }

            // 执行命令（包含文件重定向）
            execute_command(&pipeline->cmds[i], STDIN_FILENO, STDOUT_FILENO);
            exit(EXIT_FAILURE); // exec失败时退出
        } 
        else if (pid > 0) { // 父进程
            // 关闭前一个管道的读端（如果有）
            if (i > 0) {
                close(prev_pipe_read);
            }

            // 保存当前管道的读端供下一个命令使用
            if (i < pipeline->cmd_count - 1) {
                prev_pipe_read = pipefd[0];
                close(pipefd[1]);  // 关闭父进程不需要的写端
            }

            pids[pid_count++] = pid;
        } 
        else {
            perror("fork");
            exit(EXIT_FAILURE);
        }
    }

    // 等待前台进程
    if (!pipeline->background) {
        for (int i = 0; i < pid_count; i++) {
            waitpid(pids[i], NULL, 0);
        }
    }
}
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
// 原有cd命令处理保持不变...
// 新增：处理cd命令的函数
void handle_cd(char **args) {
    char *target = args[1];
    char cwd[PATH_MAX];

    // 保存当前目录作为下次的prev_dir
    if (getcwd(cwd, sizeof(cwd)) == NULL) {
        perror("getcwd");
        return;
    }

    // 处理不同参数情况
    if (!target) {  // 无参数：切换到HOME
        target = getenv("HOME");
        if (!target) {
            fprintf(stderr, "cd: HOME environment variable not set\n");
            return;
        }
    } else if (strcmp(target, "-") == 0) {  // cd -
        if (prev_dir[0] == '\0') {
            fprintf(stderr, "cd: no previous directory\n");
            return;
        }
        target = prev_dir;
    }

    // 执行目录切换
    if (chdir(target) == -1) {
        perror("cd");
    } else {
        // 更新prev_dir为切换前的目录
        strncpy(prev_dir, cwd, sizeof(prev_dir));
        prev_dir[sizeof(prev_dir)-1] = '\0';
    }
    s=malloc(sizeof(target));
    strcpy(s,target);
}

// 修改后的命令执行函数

// 新增：初始化prev_dir
void init_shell() {
    if (getcwd(prev_dir, sizeof(prev_dir)) == NULL) {
        perror("getcwd");
        exit(EXIT_FAILURE);
    }
}

int main() {
    char *line = NULL;
    size_t bufsize = 0;
    char *prompt = "shell> ";
    int arg_count=0;

    init_shell();
    signal(SIGINT,sig_catch);
    while (1) {
        
        printf("%s", prompt);
        if(s!=NULL)
        {
            printf("%s> ",s);
        }
        fflush(stdout);//用于清空（刷新）标准输出缓冲区（stdout），确保文本被立即显示

        if (getline(&line, &bufsize, stdin) == -1) {//用于存储读取的行
            break;
        }

        // 处理内建命令
        if (strncmp(line, "cd", 2) == 0) {
            char **args = parse_line(line, &arg_count);//解析参数
            handle_cd(args);
            free(args);
            continue;
        }

        if (strcmp(line, "exit\n") == 0) {
            break;
        }

        Pipeline pipeline = parse_input(line);
        
        if (pipeline.cmd_count == 0) {
            continue;
        }

        execute_pipeline(&pipeline);
    }

    free(line);
    free(s);
    return 0;
}