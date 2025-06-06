#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <pwd.h>
#include <grp.h>
/*
功能
列出目录
- 如果没有任何参数，列出当前目录
- 有 -l 选项时，列出文件权限
- 可以指定目录路径

用法示例：
$ ./ls                    # 列出当前目录
$ ./ls -l                 # 列出当前目录，显示权限
$ ./ls /                  # 列出根目录
$ ./ls /home              # 列出/home目录
$ ./ls -l /home           # 列出/home目录，显示权限
$ ./ls /usr/bin           # 列出/usr/bin目录
*/

// 返回值：0=普通列出当前目录, 1=显示权限列出当前目录, 2=列出指定目录, 3=显示权限列出指定目录
#define LIST_CURRENT_DIR 0
#define LIST_CURRENT_DIR_WITH_PERMISSION 1
#define LIST_SPECIFIC_DIR 2
#define LIST_SPECIFIC_DIR_WITH_PERMISSION 3

// 将文件权限转换为字符串格式
void get_permissions(mode_t mode, char *perm_str) {
    /*
    10位权限字符串：drwxr-xr-x
    第1位：文件类型（d=目录，-=普通文件）
    第2-4位：所有者权限（rwx）
    第5-7位：组权限（rwx）
    第8-10位：其他用户权限（rwx）
    */
    perm_str[0] = (S_ISDIR(mode)) ? 'd' : '-';
    perm_str[1] = (mode & S_IRUSR) ? 'r' : '-';
    perm_str[2] = (mode & S_IWUSR) ? 'w' : '-';
    perm_str[3] = (mode & S_IXUSR) ? 'x' : '-';
    perm_str[4] = (mode & S_IRGRP) ? 'r' : '-';
    perm_str[5] = (mode & S_IWGRP) ? 'w' : '-';
    perm_str[6] = (mode & S_IXGRP) ? 'x' : '-';
    perm_str[7] = (mode & S_IROTH) ? 'r' : '-';
    perm_str[8] = (mode & S_IWOTH) ? 'w' : '-';
    perm_str[9] = (mode & S_IXOTH) ? 'x' : '-';
    perm_str[10] = '\0';
}

// 列出目录内容
void list_directory(const char *path, int show_permissions) {
    DIR *dir;
    struct dirent *entry;
    struct stat file_stat;
    char full_path[1024];
    char perm_str[11];
    
    dir = opendir(path);
    if (dir == NULL) {
        perror("opendir");
        return;
    }
    
    while ((entry = readdir(dir)) != NULL) {
        // 跳过隐藏文件（以.开头的文件）
        if (entry->d_name[0] == '.') {
            continue;
        }
        
        if (show_permissions) {
            // 构建完整路径
            snprintf(full_path, sizeof(full_path), "%s/%s", path, entry->d_name);
            
            // 获取文件状态
            if (stat(full_path, &file_stat) == 0) {
                get_permissions(file_stat.st_mode, perm_str);
                printf("%s %s\n", perm_str, entry->d_name);
            } else {
                printf("?????????? %s\n", entry->d_name);
            }
        } else {
            printf("%s\n", entry->d_name);
        }
    }
    
    closedir(dir);
}

// 解析命令行参数
int parse_args(int argc, char *argv[], char **target_dir) {
    int show_permissions = 0;
    *target_dir = ".";  // 默认为当前目录
    
    if (argc == 1) {
        // 无参数，列出当前目录
        return LIST_CURRENT_DIR;
    }
    else if (argc == 2) {
        if (strcmp(argv[1], "-l") == 0) {
            // 只有-l参数，显示当前目录权限
            return LIST_CURRENT_DIR_WITH_PERMISSION;
        } else {
            // 指定目录
            *target_dir = argv[1];
            return LIST_SPECIFIC_DIR;
        }
    }
    else if (argc == 3) {
        if (strcmp(argv[1], "-l") == 0) {
            // -l 加目录路径
            *target_dir = argv[2];
            return LIST_SPECIFIC_DIR_WITH_PERMISSION;
        } else {
            // 参数错误
            printf("用法: %s [-l] [目录路径]\n", argv[0]);
            return -1;
        }
    }
    else {
        // 参数过多
        printf("用法: %s [-l] [目录路径]\n", argv[0]);
        return -1;
    }
}

int main(int argc, char *argv[]){
    char *target_dir;
    int flag = parse_args(argc, argv, &target_dir);
    
    if (flag == -1) {
        return 1;  // 参数错误，退出
    }
    
    switch (flag) {
        case LIST_CURRENT_DIR:
            // 列出当前目录
            list_directory(".", 0);
            break;
        case LIST_CURRENT_DIR_WITH_PERMISSION:
            // 列出当前目录，显示权限
            list_directory(".", 1);
            break;
        case LIST_SPECIFIC_DIR:
            // 列出指定目录
            list_directory(target_dir, 0);
            break;
        case LIST_SPECIFIC_DIR_WITH_PERMISSION:
            // 列出指定目录，显示权限
            list_directory(target_dir, 1);
            break;
        default:
            printf("未知错误\n");
            return 1;
    }
    
    return 0;
}
