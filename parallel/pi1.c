/*
莱布尼兹级数公式: 1 - 1/3 + 1/5 - 1/7 + 1/9 - ... = PI/4
主线程创建 1 个辅助线程
主线程计算级数的前半部分
辅助线程计算级数的后半部分
主线程等待辅助线程运行結束后,将前半部分和后半部分相加
*/
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

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