#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <limits.h>
#include <errno.h>

#define MAX_LINE 80

// 新增全局变量存储上一个目录
static char prev_dir[PATH_MAX] = {0};
char *s=NULL;
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
void execute_command(char **args) {
    // 新增：处理内建命令
    if (strcmp(args[0], "cd") == 0) {
        handle_cd(args);
        return;
    }

    pid_t pid, wpid;
    int status;

    pid = fork();
    if (pid == 0) {
        if (execvp(args[0], args) == -1) {
            perror("shell");
        }
        exit(EXIT_FAILURE);
    } else if (pid < 0) {
        perror("shell");
    } else {
        do {
            wpid = waitpid(pid, &status, WUNTRACED);
        } while (!WIFEXITED(status) && !WIFSIGNALED(status));
    }
}

// 新增：初始化prev_dir
void init_shell() {
    if (getcwd(prev_dir, sizeof(prev_dir)) == NULL) {
        perror("getcwd");
        exit(EXIT_FAILURE);
    }
}

int main() {
    char *line = NULL;
    int arg_count;
    char **args;
    size_t bufsize = 0;
    char *prompt = "shell> ";

    init_shell();  // 初始化prev_dir

    while (1) {
        printf("%s", prompt);
        if(s!=NULL)
        {
            printf("%s> ",s);
        }
        fflush(stdout);

        if (getline(&line, &bufsize, stdin) == -1) {
            break;  // 处理Ctrl+D
        }

        // // 处理空输入
        // if (strlen(line) == 1) {  // 只有换行符
        //     free(line);
        //     line = NULL;
        //     bufsize = 0;
        //     continue;
        // }

        // line[strcspn(line, "\n")] = 0;  // 去除换行符

        if (strcmp(line, "exit") == 0) {
            break;
        }

        args = parse_line(line, &arg_count);

        // 新增：处理cd参数过多的情况
        // if (strcmp(args[0], "cd") == 0 && arg_count > 2) {
        //     fprintf(stderr, "cd: too many arguments\n");
        //     free(args);
        //     continue;
        // }

        if (arg_count == 0) {
            free(args);
            continue;
        }

        execute_command(args);
        free(args);
    }

    free(line);
    return 0;
}