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

        struct cmd cmd;
        parse_cmd(line, &cmd);

        if (builtin_cmd(&cmd))
            continue;

        pid_t pid = fork();
        if (pid == 0) {
            exec_cmd(&cmd); 
        }
        wait(NULL);
    }
    return 0;
}
