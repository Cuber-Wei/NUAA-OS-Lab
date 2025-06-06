#include <stdio.h>
/*
编写程序 echo.c
打印输出命令行参数
*/
int main(int argc, char *argv[]){
    int i;
    for(i = 1; i < argc; i++){
        // 第一个参数为程序名，不打印
        printf("%s%s", argv[i], (i < argc - 1) ? " " : "");
    }
    printf("\n");
    return 0;
}