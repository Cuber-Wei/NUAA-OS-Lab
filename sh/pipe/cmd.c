#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include "cmd.h"

void exec_cmd(struct cmd *cmd)
{
    /* 在此重定向输入 */
    /* 在此重定向输出 */
    /* 完成此函数 */
    if (cmd->input) {
        int fd = open(cmd->input, O_RDONLY);
        if (fd < 0) {
            perror(cmd->input);
            exit(1);
        }
        dup2(fd, 0);
        close(fd);
    }
    // 输出重定向
    if (cmd->output) {
        int fd = open(cmd->output, O_WRONLY | O_CREAT | O_TRUNC, 0666);
        if (fd < 0) {
            perror(cmd->output);
            exit(1);
        }
        dup2(fd, 1);
        close(fd);
    }
    execvp(cmd->argv[0], cmd->argv);
    perror("exec");
    exit(0);
}

int builtin_cmd(struct cmd *cmd)
{
    /* 实现 cd 命令, 返回 1 */
    if (cmd->argc > 0 && strcmp(cmd->argv[0], "cd") == 0) {
        const char *path = cmd->argc > 1 ? cmd->argv[1] : getenv("HOME");
        if (chdir(path) != 0) {
            perror("cd");
        }
        return 1;
    }
    /* 实现 pwd 命令, 返回 1 */
    if (cmd->argc > 0 && strcmp(cmd->argv[0], "pwd") == 0) {
        char cwd[256];
        if (getcwd(cwd, sizeof(cwd)) != NULL) {
            printf("%s\n", cwd);
        } else {
            perror("pwd");
        }
        return 1;
    }
    /* 实现 exit 命令, 返回 1 */
    if (strcmp(cmd->argv[0], "exit") == 0) {
        exit(0);
        return 1;
    }
    
    return 0;
}

void exec_pipe_cmd(int cmdc, struct cmd *cmdv)
{
    if (cmdc == 1) {
        exec_cmd(&cmdv[0]);
        return;
    }

    /* 处理管道中包含 2 个命令的情况 */
    int i;
    int prev_fd = -1; // 上一个管道的读端
    int pipefd[2];

    for (i = 0; i < cmdc; i++) {
        if (i < cmdc - 1) {
            pipe(pipefd); // 创建下一个管道
        }

        pid_t pid = fork();
        if (pid == 0) {
            // 子进程
            if (i > 0) {
                // 不是第一个命令，从上一个管道读
                dup2(prev_fd, 0);
                close(prev_fd);
            }
            if (i < cmdc - 1) {
                // 不是最后一个命令，向下一个管道写
                close(pipefd[0]);
                dup2(pipefd[1], 1);
                close(pipefd[1]);
            }
            // 关闭父进程多余的 pipe
            // 递归调用时不会有多余 pipe
            exec_cmd(&cmdv[i]);
            exit(1);
        }

        // 父进程
        if (prev_fd != -1) close(prev_fd);
        if (i < cmdc - 1) {
            close(pipefd[1]);
            prev_fd = pipefd[0];
        }
    }

    // 等待所有子进程
    for (i = 0; i < cmdc; i++) {
        wait(NULL);
    }
}