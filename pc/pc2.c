/*
功能和前面的实验相同，使用信号量解决
*/
#include <stdio.h>
#include <pthread.h>
#include <ctype.h>
#include "sema.h"

#define CAPACITY 4

struct buffer
{
    int data[CAPACITY];
    int in;
    int out;
    sema_t mutex;
    sema_t empty;
    sema_t full;
};

void buffer_init(struct buffer *buffer)
{
    buffer->in = 0;
    buffer->out = 0;
    sema_init(&buffer->mutex, 1); // 初始化为1（互斥信号量）
    sema_init(&buffer->empty, CAPACITY);
    sema_init(&buffer->full, 0);
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
    sema_wait(&buffer->empty);
    sema_wait(&buffer->mutex);

    // 放入数据
    buffer->data[buffer->in] = item;
    buffer->in = (buffer->in + 1) % CAPACITY;
    
    sema_signal(&buffer->mutex);
    sema_signal(&buffer->full);
}

int buffer_get(struct buffer *buffer)
{
    sema_wait(&buffer->full);
    sema_wait(&buffer->mutex);

    // 取出数据
    int item = buffer->data[buffer->out];
    buffer->out = (buffer->out + 1) % CAPACITY;

    sema_signal(&buffer->mutex);
    sema_signal(&buffer->empty);
    return item;
}
#define ITEM_COUNT (2 * CAPACITY)

struct buffer buffer_pc, buffer_cc; // 两个缓冲区

void *produce(void *arg)
{
    for (int i = 0; i < ITEM_COUNT; i++)
    {
        int item = 'a' + i;
        // printf("produce item: %c\n", item);
        buffer_put(&buffer_pc, item);
    }
    return NULL;
}

void *compute(void *arg)
{
    for (int i = 0; i < ITEM_COUNT; i++)
    {
        int item = buffer_get(&buffer_pc);
        item = toupper(item);
        // printf("compute item: %c\n", item);
        buffer_put(&buffer_cc, item);
    }
    return NULL;
}

void *consume(void *arg)
{
    for (int i = 0; i < ITEM_COUNT; i++)
    {
        int item = buffer_get(&buffer_cc);
        printf("%c\n", item);
    }
    return NULL;
}

int main()
{
    buffer_init(&buffer_pc);
    buffer_init(&buffer_cc);

    pthread_t producer_tid, computer_tid, consumer_tid;

    pthread_create(&producer_tid, NULL, produce, NULL);
    pthread_create(&computer_tid, NULL, compute, NULL);
    pthread_create(&consumer_tid, NULL, consume, NULL);

    pthread_join(producer_tid, NULL);
    pthread_join(computer_tid, NULL);
    pthread_join(consumer_tid, NULL);

    return 0;
}