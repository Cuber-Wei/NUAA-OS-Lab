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
    /* 完成此函数 */
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
    if (cmd->argc > 0 && strcmp(cmd->argv[0], "exit") == 0) {
        exit(0);
        return 1;
    }

    return 0;
}
