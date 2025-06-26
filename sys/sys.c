#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <string.h>
/* 
    实现函数 sys

    用于执行系统命令 command
    sys 的功能与系统函数 system 相同
    参考 API

    https://man7.org/linux/man-pages/man3/system.3.html
    要求
    用进程管理相关系统调用自己实现一遍

    你的实现要使用 fork/exec/wait 系统调用
    3个都要用到
    不能调用系统函数 system 的功能

    不能调用系统程序 sh 的功能

    把 command 分割为多个单词

    保存到 words 数组中
    需要使用 strtok 函数
    fork 创建子进程

    子进程 execlp(words[0], words)
    父进程 wait 子进程结束
*/
#define MAX_WORDS 256
void sys(char *command){
    // 创建command的副本，因为strtok会修改原字符串
    char *cmd_copy = malloc(strlen(command) + 1);
    if (cmd_copy == NULL) {
        perror("malloc");
        return;
    }
    strcpy(cmd_copy, command);
    
    // 分割命令为单词数组
    char *words[MAX_WORDS];
    int word_count = 0;
    
    // 使用strtok分割字符串
    char *token = strtok(cmd_copy, " \t\n");
    while (token != NULL && word_count < MAX_WORDS - 1) {
        words[word_count++] = token;
        token = strtok(NULL, " \t\n");
    }
    words[word_count] = NULL;  // execvp需要NULL结尾的数组
    
    // 如果没有找到任何单词，直接返回
    if (word_count == 0) {
        free(cmd_copy);
        return;
    }
    
    // fork创建子进程
    pid_t pid = fork();
    
    if (pid == 0) {
        // 子进程：执行命令
        // 使用execvp而不是execlp，因为我们有参数数组
        execvp(words[0], words);
        
        // 如果execvp失败，打印错误并退出
        perror("execvp");
        exit(1);
    }
    else if (pid < 0) {
        // fork失败
        perror("fork");
        free(cmd_copy);
        return;
    }
    else {
        // 父进程：等待子进程结束
        int status;
        waitpid(pid, &status, 0);
    }
    
    // 清理内存
    free(cmd_copy);
}

int main(){
    /* 不要修改 main 函数 */
    puts("---");
    sys("echo HELLO WORLD");
    puts("---");
    sys("ls /");
    puts("---");
    return 0;
}