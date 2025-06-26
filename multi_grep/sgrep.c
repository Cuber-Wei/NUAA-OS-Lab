#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>

void grep_file(char *path, char *target)
{
    FILE *file = fopen(path, "r");

    char line[256];
    while (fgets(line, sizeof(line), file)) { 
        if (strstr(line, target))
            printf("%s:%s", path, line);
    }

    fclose(file);
}

void grep_dir(char *path, char *target)
{
    DIR *dir = opendir(path);
    struct dirent *entry;
    while (entry = readdir(dir)) {
        if (strcmp(entry->d_name, ".") == 0) 
            continue;

        if (strcmp(entry->d_name, "..") == 0) 
            continue;

        char sub_path[300];
        sprintf(sub_path, "%s/%s", path, entry->d_name);

        if (entry->d_type == DT_DIR) 
            grep_dir(sub_path, target);

        if (entry->d_type == DT_REG) 
            grep_file(sub_path, target);
    }
    closedir(dir);
}

void usage()
{
    puts("Usage:");
    puts(" sgrep string path");
    puts(" sgrep -r string path");
}

int main(int argc, char *argv[])
{
    if (argc < 3)
        usage();

    char *string = argv[argc - 2];
    char *path = argv[argc - 1];
    if (strcmp(argv[1], "-r") == 0)
        grep_dir(path, string);
    else
        grep_file(path, string);
    return 0;
}
