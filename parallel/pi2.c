/*
与上一题类似，但本题更加通用化，能适应 N 个核心
主线程创建 N 个辅助线程
每个辅助线程计算一部分任务，并将结果返回
主线程等待 N 个辅助线程运行结束，将所有辅助线程的结果累加
本题要求 1: 使用线程参数，消除程序中的代码重复
本题要求 2: 不能使用全局变量存储线程返回值
*/
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