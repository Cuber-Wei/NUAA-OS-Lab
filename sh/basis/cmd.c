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
