# 操作系统课设报告

[TOC]

## shell 的实现
> 由于本题由`basis`发展而来，故关键代码分析基于`pipe`中的代码。

将输入重定向到当前进程的标准输入，将当前进程标准输出重定向到指定输出。

```c
// cmd.c
void exec_cmd(struct cmd *cmd)
{
    // 输入重定向
    if (cmd->input) {
        int fd = open(cmd->input, O_RDONLY);
        if (fd < 0) {
            perror(cmd->input);
            exit(1);
        }
        dup2(fd, 0);
        close(fd);
    }
    // 输出重定向
    if (cmd->output) {
        int fd = open(cmd->output, O_WRONLY | O_CREAT | O_TRUNC, 0666);
        if (fd < 0) {
            perror(cmd->output);
            exit(1);
        }
        dup2(fd, 1);
        close(fd);
    }
    execvp(cmd->argv[0], cmd->argv);
    perror("exec");
    exit(0);
}
```

通过特判拦截`cd`，`pwd`，`exit`命令，并加以实现。

```c
// cmd.c
int builtin_cmd(struct cmd *cmd)
{
    /* 实现 cd 命令, 返回 1 */
    if (cmd->argc > 0 && strcmp(cmd->argv[0], "cd") == 0) {
        const char *path = cmd->argc > 1 ? cmd->argv[1] : getenv("HOME");
        if (chdir(path) != 0) {
            perror("cd");
        }
        return 1;
    }
    /* 实现 pwd 命令, 返回 1 */
    if (cmd->argc > 0 && strcmp(cmd->argv[0], "pwd") == 0) {
        char cwd[256];
        if (getcwd(cwd, sizeof(cwd)) != NULL) {
            printf("%s\n", cwd);
        } else {
            perror("pwd");
        }
        return 1;
    }
    /* 实现 exit 命令, 返回 1 */
    if (strcmp(cmd->argv[0], "exit") == 0) {
        exit(0);
        return 1;
    }
    
    return 0;
}
```

通过`for`循环遍历每个需要执行的进程，在进程交界处设置管道连接前后两个进程的输入和输出。

```c
// cmd.c
void exec_pipe_cmd(int cmdc, struct cmd *cmdv)
{
    if (cmdc == 1) {
        exec_cmd(&cmdv[0]);
        return;
    }

    /* 处理管道中包含 2 个命令的情况 */
    int i;
    int prev_fd = -1; // 上一个管道的读端
    int pipefd[2];

    for (i = 0; i < cmdc; i++) {
        if (i < cmdc - 1) {
            pipe(pipefd); // 创建下一个管道
        }

        pid_t pid = fork();
        if (pid == 0) {
            // 子进程
            if (i > 0) {
                // 不是第一个命令，从上一个管道读
                dup2(prev_fd, 0);
                close(prev_fd);
            }
            if (i < cmdc - 1) {
                // 不是最后一个命令，向下一个管道写
                close(pipefd[0]);
                dup2(pipefd[1], 1);
                close(pipefd[1]);
            }
            // 关闭父进程多余的 pipe
            // 递归调用时不会有多余 pipe
            exec_cmd(&cmdv[i]);
            exit(1);
        }

        // 父进程
        if (prev_fd != -1) close(prev_fd);
        if (i < cmdc - 1) {
            close(pipefd[1]);
            prev_fd = pipefd[0];
        }
    }

    // 等待所有子进程
    for (i = 0; i < cmdc; i++) {
        wait(NULL);
    }
}
```

通过`strtok`获取token，特判重定向运算符进入管道的输入输出配置逻辑。
`>`（输出重定向）需要配置输出项（`cmd->input`）；`<`（输入重定向）需要配置输入项。（`cmd->output`）

```c
// parse.c
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
```

管道运算符的解析，将管道解析成各个进程的参数。

```c
// parse.c
int parse_pipe_cmd(char *line, struct cmd *cmdv)
{
    /* 使用 strtok_r 完成此函数 */
    int cmdc = 0;
    char *saveptr;
    char *segment = strtok_r(line, "|", &saveptr);
    while (segment != NULL) {
        // 去除前后空格
        while (*segment == ' ') segment++; // 去除前空格
        char *end = segment + strlen(segment) - 1;
        while (end > segment && *end == ' ') {
            *end = '\0';
            end--;
        }
        parse_cmd(segment, &cmdv[cmdc]);
        cmdc++;
        segment = strtok_r(NULL, "|", &saveptr);
    }
    return cmdc;
}
```
## 并行编程
```c
#define N 1000000 // 级数项数

double sum1 = 0.0; // 主线程计算的前半部分和
double sum2 = 0.0; // 辅助线程计算的后半部分
pthread_mutex_t mutex; // 互斥锁

void *calculate_partial_sum(void *arg) {
    for (int i = N / 2; i < N; i++) {
        if (i % 2 == 0) {
            sum2 += 1.0 / (2 * i + 1);
        } else {
            sum2 -= 1.0 / (2 * i + 1);
        }
    }
    return NULL;
}

int main() {
    pthread_t thread;
    pthread_mutex_init(&mutex, NULL); // 初始化互斥锁

    // 创建辅助线程
    if (pthread_create(&thread, NULL, calculate_partial_sum, NULL) != 0) {
        perror("Failed to create thread");
        return 1;
    }

    // 主线程计算前半部分和
    for (int i = 0; i < N / 2; i++) {
        if (i % 2 == 0) {
            sum1 += 1.0 / (2 * i + 1);
        } else {
            sum1 -= 1.0 / (2 * i + 1);
        }
    }

    // 等待辅助线程结束
    pthread_join(thread, NULL);

    // 合并结果
    double pi = (sum1 + sum2) * 4.0;

    printf("%.15f\n", pi);

    pthread_mutex_destroy(&mutex); // 销毁互斥锁
    return 0;
}
```

通过for循环构造N个线程（N个核心），每个线程都分别计算一部分级数和，保存到各自的结果地址中。最后由主进程进行整合计算。

```c
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#define N 1000000 // 级数项数
#define THREAD_COUNT 4 // 线程数量

double sum[THREAD_COUNT] = {0.0}; // 每个线程的部分和
pthread_mutex_t mutex; // 互斥锁

void *calculate_partial_sum(void *arg) {
    int thread_id = *(int *)arg; // 获取线程 ID
    int start = thread_id * (N / THREAD_COUNT);
    int end = (thread_id + 1) * (N / THREAD_COUNT);
    
    for (int i = start; i < end; i++) {
        if (i % 2 == 0) {
            sum[thread_id] += 1.0 / (2 * i + 1);
        } else {
            sum[thread_id] -= 1.0 / (2 * i + 1);
        }
    }
    return NULL;
}

int main() {
    pthread_t threads[THREAD_COUNT];
    pthread_mutex_init(&mutex, NULL); // 初始化互斥锁
    int thread_ids[THREAD_COUNT]; // 线程 ID 数组

    // 创建辅助线程
    for (int i = 0; i < THREAD_COUNT; i++) {
        thread_ids[i] = i; // 设置线程 ID
        if (pthread_create(&threads[i], NULL, calculate_partial_sum, &thread_ids[i]) != 0) {
            perror("Failed to create thread");
            return 1;
        }
    }

    // 等待所有辅助线程结束
    for (int i = 0; i < THREAD_COUNT; i++) {
        pthread_join(threads[i], NULL);
    }

    // 合并结果
    double pi = 0.0;
    for (int i = 0; i < THREAD_COUNT; i++) {
        pi += sum[i];
    }
    pi *= 4.0;

    printf("%.15f\n", pi);

    pthread_mutex_destroy(&mutex); // 销毁互斥锁
    return 0;
}
```

实现选择排序函数，利用for循环创建两个子线程，分别排序一部分的数组元素。由主进程完成两部分有序的数组合并。

```c
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#define MAX_SIZE 1000

typedef struct {
    int *array;
    int start;
    int end;
} SortArgs;

void selection_sort(SortArgs *args)
{
    int *array = args->array;
    int start = args->start;
    int end = args->end;

    // 检查参数有效性
    if (start < 0 || end > MAX_SIZE || start >= end) {
        fprintf(stderr, "Invalid sorting range: %d to %d\n", start, end);
        return;
    }

    for (int i = start; i < end - 1; i++) {
        int min_index = i;
        for (int j = i + 1; j < end; j++) {
            if (array[j] < array[min_index]) {
                min_index = j;
            }
        }
        if (min_index != i) {
            int temp = array[i];
            array[i] = array[min_index];
            array[min_index] = temp;
        }
    }
}

void merge(int *array, int size)
{
    int mid = size / 2;
    int left[mid], right[size - mid];

    for (int i = 0; i < mid; i++)
        left[i] = array[i];
    for (int i = mid; i < size; i++)
        right[i - mid] = array[i];

    int i = 0, j = 0, k = 0;
    while (i < mid && j < size - mid) {
        if (left[i] <= right[j]) {
            array[k++] = left[i++];
        } else {
            array[k++] = right[j++];
        }
    }
    while (i < mid) {
        array[k++] = left[i++];
    }
    while (j < size - mid) {
        array[k++] = right[j++];
    }
}

void parallel_sort(int *array, int size)
{
    pthread_t threads[2];
    SortArgs args[2];

    // 设置参数
    args[0].array = array;
    args[0].start = 0;
    args[0].end = size / 2;

    args[1].array = array;
    args[1].start = size / 2;
    args[1].end = size;

    // 创建线程
    pthread_create(&threads[0], NULL, (void *(*)(void *))selection_sort, &args[0]);
    pthread_create(&threads[1], NULL, (void *(*)(void *))selection_sort, &args[1]);

    // 等待线程结束
    pthread_join(threads[0], NULL);
    pthread_join(threads[1], NULL);

    // 合并结果
    merge(array, size);
}

int main()
{
    int array[8] = {3, 1, 2, 4, 5, 6, 7, 0};
    parallel_sort(array, 8);
    int i;
    for (i = 0; i < 8; i++)
        printf("%d\n", array[i]);
    return 0;
}
```

## 生产者消费者
标准的`生产者-消费者`模型的实现。
```c
#include <stdio.h>
#include <pthread.h>
#include <ctype.h>

#define CAPACITY 4

struct buffer
{
    int data[CAPACITY];
    int in;  // 当前操作添加的元素位置
    int out; // 当前操作取出的元素位置
    /*
    在操作前判断
    full： 下一操作添加的元素位置在当前操作即将取出的元素位置(in + 1 == out)
    empty：当前操作添加的的元素位置当前操作取出的元素位置(in == out)
    */
    pthread_mutex_t mutex;
    pthread_cond_t wait_empty_buffer;
    pthread_cond_t wait_full_buffer;
};

void buffer_init(struct buffer *buffer)
{
    buffer->in = 0;
    buffer->out = 0;
    pthread_mutex_init(&buffer->mutex, NULL);
    pthread_cond_init(&buffer->wait_empty_buffer, NULL);
    pthread_cond_init(&buffer->wait_full_buffer, NULL);
}

int buffer_is_empty(struct buffer *buffer)
{
    return buffer->in == buffer->out;
}

int buffer_is_full(struct buffer *buffer)
{
    return (buffer->in + 1) % CAPACITY == buffer->out;
}

void buffer_put(struct buffer *buffer, int item)
{
    pthread_mutex_lock(&buffer->mutex);
    while (buffer_is_full(buffer))
        pthread_cond_wait(&buffer->wait_empty_buffer, &buffer->mutex);

    buffer->data[buffer->in] = item;
    buffer->in = (buffer->in + 1) % CAPACITY;

    pthread_cond_signal(&buffer->wait_full_buffer);
    pthread_mutex_unlock(&buffer->mutex);
}

int buffer_get(struct buffer *buffer)
{
    int item = -1;
    pthread_mutex_lock(&buffer->mutex);
    while (buffer_is_empty(buffer))
        pthread_cond_wait(&buffer->wait_full_buffer, &buffer->mutex);

    item = buffer->data[buffer->out];
    buffer->out = (buffer->out + 1) % CAPACITY;

    pthread_cond_signal(&buffer->wait_empty_buffer);
    pthread_mutex_unlock(&buffer->mutex);
    return item;
}
```

在`sema.h`中对记录型信号量操作进行实现。

```c
#include <stdio.h>
#include <pthread.h>
#include <ctype.h>
#include "sema.h"

#define CAPACITY 4

struct buffer
{
    int data[CAPACITY]; // buffer中的数据队列
    int in; // buffer写端
    int out; // buffer读端
    sema_t mutex; // 互斥访问锁
    sema_t empty; // 剩余数量
    sema_t full; // 正在拿数据的进程数量
};

void buffer_init(struct buffer *buffer)
{
    buffer->in = 0;
    buffer->out = 0;
    sema_init(&buffer->mutex, 1); // 初始化为1（互斥信号量）
    sema_init(&buffer->empty, CAPACITY); // 剩余数量初始化为最大容量
    sema_init(&buffer->full, 0); // 无进程正在拿取数据
}

void buffer_put(struct buffer *buffer, int item)
{
    sema_wait(&buffer->empty); // 消耗一个容量
    sema_wait(&buffer->mutex); // 互斥访问

    // 放入数据
    buffer->data[buffer->in] = item;
    buffer->in = (buffer->in + 1) % CAPACITY;
    
    sema_signal(&buffer->mutex); // 交回访问权
    sema_signal(&buffer->full); // 唤醒一个拿取数据进程
}

int buffer_get(struct buffer *buffer)
{
    sema_wait(&buffer->full); // 消耗一个拿取数据进程数
    sema_wait(&buffer->mutex); // 互斥访问

    // 取出数据
    int item = buffer->data[buffer->out];
    buffer->out = (buffer->out + 1) % CAPACITY;

    sema_signal(&buffer->mutex); // 交回访问权
    sema_signal(&buffer->empty); // 唤醒一个放入数据进程
    return item;
}
// sema.h
#include <pthread.h>

typedef struct {
    int value;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
} sema_t;

void sema_init(sema_t *sema, int value)
{
    // 初始化记录型信号量的初值，并初始化互斥锁和条件变量
    sema->value = value;
    pthread_mutex_init(&sema->mutex, NULL);
    pthread_cond_init(&sema->cond, NULL);
}

void sema_wait(sema_t *sema)
{
    // 互斥访问value的值
    pthread_mutex_lock(&sema->mutex);
    while (sema->value <= 0) // 防止越界
        pthread_cond_wait(&sema->cond, &sema->mutex);
    sema->value--; // 消耗一个value
    pthread_mutex_unlock(&sema->mutex); //归还访问权限
}

void sema_signal(sema_t *sema)
{
    // 互斥访问value的值
    pthread_mutex_lock(&sema->mutex);
    ++sema->value; // 新增一个value
    pthread_cond_signal(&sema->cond); // 唤醒一个等待的线程
    pthread_mutex_unlock(&sema->mutex); // 归还访问权限
}

```
## 文件系统
主要利用库函数进行实际操作。
```c
// main.c
int uxfs_read(const char *path, char *buf, size_t size, off_t offset,
             struct fuse_file_info *info)
{
    log_printf("read %s %ld at %d\n", path, size, offset);
    /* TODO */
    inode_t *leaf = (inode_t *)(intptr_t)(info->fh); // 获取目标节点的inode
    int count = inode_read(leaf, offset, buf, size); // 阅读目标节点(leaf)目标偏移(offset)目标大小(size)到buf中
    return count;
}

int uxfs_write(const char *path, const char *buf, size_t size, off_t offset,
              struct fuse_file_info *info)
{
    log_printf("write %s at %d\n", path, offset);
    /* TODO */
    int count = 0;
    inode_t *leaf = (inode_t *)(intptr_t)(info->fh); // 获取目标节点的inode 
    count = inode_write(leaf, offset, (char *)buf, size); // 将buff中的数据写入到目标节点(leaf)目标偏移(offset)目标大小(size)中
    return count;
}

int uxfs_readdir(const char *path, void *buf, fuse_fill_dir_t fill, 
                  off_t offset, struct fuse_file_info *info)
{
    inode_t *dir = (inode_t *)(intptr_t)(info->fh);
    log_printf("readdir %s\n", path);

    dir_cursor_t dir_cursor;
    dir_cursor_init(&dir_cursor, dir);

    /* TODO */
    fill(buf, ".", NULL, 0); // 补充隐藏目录 .
    fill(buf, "..", NULL, 0); // 补充隐藏目录 ..
    dir_entry_t *entry_ptr;
    while ((entry_ptr = dir_cursor_next(&dir_cursor)) != NULL) { // 遍历当前所有路径
        fill(buf, entry_ptr->name, NULL, 0); // 填充路径命到当前目录中
    }

    dir_cursor_destroy(&dir_cursor);
    return 0;
}

// dump.c
void dump_regular(inode_t *inode)
{
    /* TODO */
    char buf[101] = {0};
    int len = inode_read(inode, 0, buf, 100); // 读入文件描述信息
    buf[len > 0 ? len : 0] = '\0'; // 及时截断，防止内存泄露
    typer_dump("reg %s\n", buf); // 输出文件信息
}

void dump_dir(inode_t *dir)
{
    typer_dump("dir\n");
    typer_push();

    dir_entry_t *dir_entry;
    dir_cursor_t dir_cursor;
    dir_cursor_init(&dir_cursor, dir);

    /* TODO */
    while ((dir_entry = dir_cursor_next(&dir_cursor)) != NULL) { // 遍历当前路径
        dump_dir_entry(dir_entry); // 递归遍历目录
    }

    dir_cursor_destroy(&dir_cursor);
    typer_pop();
}

void dump_dir_entry(dir_entry_t *dir_entry)
{
    char *name = dir_entry->name;
    int ino = dir_entry->ino;
    if (ino == FAKE_INO)
        return;

    typer_dump("entry\n");
    typer_push();

    typer_dump("name %s\n", name);
    typer_dump("ino %d\n", ino);

    /* TODO */
    inode_t *inode = fs_seek_inode(ino); // 获取到ino对应的inode节点
    if (inode_is_dir(inode)) {
        // 只对非"."和".."递归
        if (!is_dot(name)) {
            dump_dir(inode);
        }
    } else if (inode_is_regular(inode)) {
        dump_regular(inode); // 打印regular文件信息
    }

    typer_pop();
}

```

