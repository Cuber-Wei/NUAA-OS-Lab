/*
功能和前面的实验相同，使用信号量解决
*/
#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <ctype.h>

#define CAPACITY 4

struct buffer
{
    int data[CAPACITY];
    int in;
    int out;
    sem_t mutex;
    sem_t empty;
    sem_t full;
};

void buffer_init(struct buffer *buffer)
{
    buffer->in = 0;
    buffer->out = 0;
    sem_init(&buffer->mutex, 0, 1); // 初始化为1（互斥信号量）
    sem_init(&buffer->empty, 0, 1);
    sem_init(&buffer->full, 0, 0);
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
    sem_wait(&buffer->mutex);
    while (buffer_is_full(buffer)) // 如果缓冲区已满
    {
        sem_post(&buffer->mutex);
        sem_wait(&buffer->empty); // 等待空槽位
        sem_wait(&buffer->mutex);
    }

    // 放入数据
    buffer->data[buffer->in] = item;
    buffer->in = (buffer->in + 1) % CAPACITY;

    sem_post(&buffer->full);
    sem_post(&buffer->mutex);
}

int buffer_get(struct buffer *buffer)
{
    sem_wait(&buffer->mutex);
    while (buffer_is_empty(buffer))
    {
        sem_post(&buffer->mutex);
        sem_wait(&buffer->full); // 等待满槽位
        sem_wait(&buffer->mutex);
    }

    // 取出数据
    int item = buffer->data[buffer->out];
    buffer->out = (buffer->out + 1) % CAPACITY;

    sem_post(&buffer->empty);
    sem_post(&buffer->mutex);
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
    // 销毁信号量
    sem_destroy(&buffer_pc.mutex);
    sem_destroy(&buffer_pc.empty);
    sem_destroy(&buffer_pc.full);
    sem_destroy(&buffer_cc.mutex);
    sem_destroy(&buffer_cc.empty);
    sem_destroy(&buffer_cc.full);

    return 0;
}