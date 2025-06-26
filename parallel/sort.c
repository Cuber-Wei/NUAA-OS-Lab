/*
主线程创建两个辅助线程
辅助线程1使用选择排序算法对数组的前半部分排序
辅助线程2使用选择排序算法对数组的后半部分排序
主线程等待辅助线程运行結束后, 使用归并排序算法归并子线程的计算结果
本题要求 1: 使用线程参数，消除程序中的代码重复
本题要求 2: 不能使用全局变量存储线程返回值

不要修改 main 函数
实现 parallel_sort 函数，可以定义新的函数
不准使用全局变量
*/

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