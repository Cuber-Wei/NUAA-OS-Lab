#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "parse.h"

void parse_cmd(char *line, struct cmd *cmd)
{
    cmd->argc = 0;
    cmd->input = NULL;
    cmd->output = NULL;

    /* 使用 strtok 完成此函数 */
    /* 完成此函数，要处理 > < 的情况 */ 
    char *token = strtok(line, " ");
    while (token != NULL) {
        if (strcmp(token, "<") == 0) {
            token = strtok(NULL, " ");
            if (token != NULL) {
                cmd->input = token;
                token = strtok(NULL, " ");
            }
        } else if (strcmp(token, ">") == 0) {
            token = strtok(NULL, " ");
            if (token != NULL) {
                cmd->output = token;
                token = strtok(NULL, " ");
            }
        } else {
            cmd->argv[cmd->argc++] = token;
            token = strtok(NULL, " ");
        }
    }

    cmd->argv[cmd->argc] = NULL;
}

void dump_cmd(struct cmd *cmd)
{
    printf("argc = %d\n", cmd->argc);
    int i;
    for (i = 0; i < cmd->argc; i++) {
        printf("argv[%d] = (%s)\n", i, cmd->argv[i]);
    }

    char *input = cmd->input ? cmd->input : "NULL";
    printf("input = (%s)\n", input);

    char *output = cmd->output ? cmd->output : "NULL";
    printf("output = (%s)\n", output);
}
