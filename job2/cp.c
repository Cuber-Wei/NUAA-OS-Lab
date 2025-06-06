#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/*
功能
复制单个文件
不要求复制目录
cc -o cp cp.c
$ ./cp cp.c cp.bak
$ cat cp.bak
#include <stdio.h>
#include <unistd.h>

int main()
{
    return 0;
}
*/
#define BUFFER_SIZE 1024

int main(int argc, char *argv[]){
    if (argc < 3){
        if (argc == 2){
            printf("请指定要复制到的文件名！\n");
        }
        else {
            printf("请指定文件名称！\n");
        }
        exit(0);
    }
    char* origin_file = argv[1];
    char* target_file = argv[2];
    FILE *origin_fd = fopen(origin_file, "r");
    if (origin_fd == NULL){
        printf("打开文件 %s 失败！\n", origin_file);
        exit(0);
    }
    FILE *target_fd = fopen(target_file, "w");
    if (target_fd == NULL){
        printf("打开文件 %s 失败！\n", target_file);
        exit(0);
    }
    char buf[BUFFER_SIZE];
    while(1){
        memset(buf, 0, BUFFER_SIZE);
        int tmp = fread(buf, 1, sizeof(buf), origin_fd);
        if (tmp == 0){
            break;
        }
        fwrite(buf, 1, tmp, target_fd);
    }
    fclose(origin_fd);
    fclose(target_fd);
    return 0;
}