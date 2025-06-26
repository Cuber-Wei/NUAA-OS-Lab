/*
实现并行的 grep

- 假设目录下文本文件的数量很多
    通过使用多个线程进行并行搜索
- 基于生产者和消费者模型
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <pthread.h>

#define MAX_QUEUE 1024
#define MAX_PATH 300
#define THREAD_NUM 4

typedef struct {
    char paths[MAX_QUEUE][MAX_PATH];
    int front, rear, count;
    pthread_mutex_t mutex;
    pthread_cond_t not_empty, not_full;
    int finished; // 标记生产者是否结束
} task_queue_t;

task_queue_t queue;
char *g_target = NULL;

void queue_init(task_queue_t *q) {
    q->front = q->rear = q->count = 0;
    pthread_mutex_init(&q->mutex, NULL);
    pthread_cond_init(&q->not_empty, NULL);
    pthread_cond_init(&q->not_full, NULL);
    q->finished = 0;
}

void queue_destroy(task_queue_t *q) {
    pthread_mutex_destroy(&q->mutex);
    pthread_cond_destroy(&q->not_empty);
    pthread_cond_destroy(&q->not_full);
}

void queue_push(task_queue_t *q, const char *path) {
    pthread_mutex_lock(&q->mutex);
    while (q->count == MAX_QUEUE)
        pthread_cond_wait(&q->not_full, &q->mutex);
    strncpy(q->paths[q->rear], path, MAX_PATH-1);
    q->paths[q->rear][MAX_PATH-1] = '\0';
    q->rear = (q->rear + 1) % MAX_QUEUE;
    q->count++;
    pthread_cond_signal(&q->not_empty);
    pthread_mutex_unlock(&q->mutex);
}

int queue_pop(task_queue_t *q, char *out) {
    pthread_mutex_lock(&q->mutex);
    while (q->count == 0 && !q->finished)
        pthread_cond_wait(&q->not_empty, &q->mutex);
    if (q->count == 0 && q->finished) {
        pthread_mutex_unlock(&q->mutex);
        return 0; // 没有任务且生产结束
    }
    strncpy(out, q->paths[q->front], MAX_PATH);
    q->front = (q->front + 1) % MAX_QUEUE;
    q->count--;
    pthread_cond_signal(&q->not_full);
    pthread_mutex_unlock(&q->mutex);
    return 1;
}

void grep_file(const char *path, const char *target) {
    FILE *file = fopen(path, "r");
    if (!file) {
        //perror("fopen");
        return;
    }
    char line[256];
    while (fgets(line, sizeof(line), file)) {
        if (strstr(line, target))
            printf("%s:%s", path, line);
    }
    fclose(file);
}

void *worker(void *arg) {
    char path[MAX_PATH];
    while (queue_pop(&queue, path)) {
        grep_file(path, g_target);
    }
    return NULL;
}

void grep_dir(const char *path) {
    DIR *dir = opendir(path);
    if (!dir) {
        //perror("opendir");
        return;
    }
    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;
        char sub_path[MAX_PATH];
        snprintf(sub_path, MAX_PATH, "%s/%s", path, entry->d_name);
        struct stat st;
        if (stat(sub_path, &st) == 0) {
            if (S_ISDIR(st.st_mode)) {
                grep_dir(sub_path);
            } else if (S_ISREG(st.st_mode)) {
                queue_push(&queue, sub_path);
            }
        }
    }
    closedir(dir);
}

void usage() {
    puts("Usage:");
    puts(" pgrep string path");
    puts(" pgrep -r string path");
}

int main(int argc, char *argv[]) {
    if (argc < 3) {
        usage();
        return 1;
    }
    char *string = argv[argc - 2];
    char *path = argv[argc - 1];
    if (strcmp(argv[1], "-r") == 0) {
        queue_init(&queue);
        g_target = string;
        pthread_t tids[THREAD_NUM];
        for (int i = 0; i < THREAD_NUM; ++i)
            pthread_create(&tids[i], NULL, worker, NULL);
        grep_dir(path); // 生产者递归遍历目录
        pthread_mutex_lock(&queue.mutex);
        queue.finished = 1;
        pthread_cond_broadcast(&queue.not_empty);
        pthread_mutex_unlock(&queue.mutex);
        for (int i = 0; i < THREAD_NUM; ++i)
            pthread_join(tids[i], NULL);
        queue_destroy(&queue);
    } else {
        grep_file(path, string);
    }
    return 0;
}
