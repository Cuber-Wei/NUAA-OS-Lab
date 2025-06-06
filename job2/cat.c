#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
/*
功能
打印指定文件的内容
*/
#define BUFFER_SIZE 1024

int main(int argc, char *argv[]){
    if (argc < 2){
        printf("请指定文件名称！\n");
        exit(0);
    }
    char* file_name = argv[1];
    FILE *fd = fopen(file_name, "r");
    if (fd == NULL){
        printf("打开文件失败！\n");
        exit(0);
    }
    char buf[1024];
    while(1){
        memset(buf, 0, BUFFER_SIZE);
        int tmp = fread(buf, 1, sizeof(buf), fd);
        if (tmp == 0){
            break;
        }
        printf("%s\n", buf);
    }
    fclose(fd);
    return 0;
}