#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>

/*
功能
打印包含指定字符串的行
不要求实现正则匹配
要求实现 -r 的功能
使用 fgets 逐行读取

需要使用的 API
opendir
readdir
closedir
stat
fgets

grep -r 目录
-r 选项
用于递归地搜索指定目录
及其所有子目录中的文件
查找 nuaaos 目录下
所有查找包含有字符串 main 的行
打印输出 2 列
完整的文件路径
匹配的行

用法示例：
$ ./grep -r 字符串 目录     # 递归搜索
$ ./grep 字符串 文件       # 搜索单个文件
*/

// 搜索单个文件中包含指定字符串的行
void search_in_file(const char *filepath, const char *search_str) {
    FILE *file = fopen(filepath, "r");
    if (file == NULL) {
        return; // 无法打开文件，跳过
    }
    
    char line[1024];
    int line_number = 1;
    
    while (fgets(line, sizeof(line), file) != NULL) {
        // 检查行中是否包含搜索字符串
        if (strstr(line, search_str) != NULL) {
            // 移除行末的换行符
            int len = strlen(line);
            if (len > 0 && line[len-1] == '\n') {
                line[len-1] = '\0';
            }
            
            printf("%s[%d]:%s\n", filepath, line_number, line);
        }
        line_number++;
    }
    
    fclose(file);
}

// 检查文件是否为普通文件且可读
int is_regular_file(const char *path) {
    struct stat file_stat;
    if (stat(path, &file_stat) != 0) {
        return 0;
    }
    return S_ISREG(file_stat.st_mode);
}

// 检查是否为目录
int is_directory(const char *path) {
    struct stat file_stat;
    if (stat(path, &file_stat) != 0) {
        return 0;
    }
    return S_ISDIR(file_stat.st_mode);
}

// 递归搜索目录
void search_directory_recursive(const char *dir_path, const char *search_str) {
    DIR *dir;
    struct dirent *entry;
    char full_path[1024];
    
    dir = opendir(dir_path);
    if (dir == NULL) {
        printf("错误：无法打开目录 %s\n", dir_path);
        return;
    }
    
    while ((entry = readdir(dir)) != NULL) {
        // 跳过 . 和 .. 目录
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }
        
        // 构建完整路径
        snprintf(full_path, sizeof(full_path), "%s/%s", dir_path, entry->d_name);
        
        if (is_regular_file(full_path)) {
            // 如果是普通文件，搜索其内容
            search_in_file(full_path, search_str);
        } else if (is_directory(full_path)) {
            // 如果是目录，递归搜索
            search_directory_recursive(full_path, search_str);
        }
    }
    
    closedir(dir);
}

// 解析命令行参数
int parse_arguments(int argc, char *argv[], int *recursive, char **search_str, char **target) {
    if (argc < 3) {
        printf("用法: %s [-r] 搜索字符串 文件/目录\n", argv[0]);
        printf("  -r: 递归搜索目录\n");
        printf("示例:\n");
        printf("  %s main file.c           # 在file.c中搜索'main'\n", argv[0]);
        printf("  %s -r main ./            # 在当前目录递归搜索'main'\n", argv[0]);
        return 0;
    }
    
    *recursive = 0;
    
    if (argc == 4 && strcmp(argv[1], "-r") == 0) {
        // 递归模式: ./grep -r 字符串 目录
        *recursive = 1;
        *search_str = argv[2];
        *target = argv[3];
    } else if (argc == 3) {
        // 普通模式: ./grep 字符串 文件
        *search_str = argv[1];
        *target = argv[2];
    } else {
        printf("参数错误\n");
        printf("用法: %s [-r] 搜索字符串 文件/目录\n", argv[0]);
        return 0;
    }
    
    return 1;
}

int main(int argc, char *argv[]){
    int recursive;
    char *search_str;
    char *target;
    
    // 解析命令行参数
    if (!parse_arguments(argc, argv, &recursive, &search_str, &target)) {
        return 1;
    }
    
    printf("搜索字符串: '%s'\n", search_str);
    printf("目标: %s\n", target);
    printf("递归模式: %s\n", recursive ? "是" : "否");
    printf("========================================\n");
    
    if (recursive) {
        // 递归搜索目录
        if (is_directory(target)) {
            search_directory_recursive(target, search_str);
        } else {
            printf("错误：%s 不是一个目录\n", target);
            return 1;
        }
    } else {
        // 搜索单个文件
        if (is_regular_file(target)) {
            search_in_file(target, search_str);
        } else if (is_directory(target)) {
            printf("错误：%s 是一个目录，请使用 -r 选项进行递归搜索\n", target);
            return 1;
        } else {
            printf("错误：%s 不是一个有效的文件\n", target);
            return 1;
        }
    }
    
    return 0;
}