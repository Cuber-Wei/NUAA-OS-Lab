#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include "cmd.h"
#include "parse.h"

void read_line(char *line, int size)
{
    char *result = fgets(line, size, stdin);
    if (result == NULL)
        exit(0);

    int len = strlen(line);
    assert(line[len - 1] == '\n');
    line[len - 1] = 0;
}

int main(int argc, char *argv[])
{
    while (1) {
        char line[100];
        write(1, "> ", 2);
        read_line(line, sizeof(line));

        int cmdc;
        struct cmd cmdv[MAX_CMDC];
        cmdc = parse_pipe_cmd(line, cmdv);

        if (builtin_cmd(&cmdv[0]))
            continue;

        pid_t pid = fork();
        if (pid == 0) {
            exec_pipe_cmd(cmdc, cmdv);
        }
        wait(NULL);
    }
    return 0;
}
