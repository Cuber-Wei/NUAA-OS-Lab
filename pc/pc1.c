/*
功能要求
系统中有3个线程：生产者、计算者、消费者
系统中有2个容量为4的缓冲区：buffer1、buffer2
生产者生产'a'、'b'、'c'、‘d'、'e'、'f'、'g'、'h'八个字符，放入到buffer1
计算者从buffer1取出字符，将小写字符转换为大写字符，放入到buffer2
消费者从buffer2取出字符，将其打印到屏幕上

特别注意
main 函数
使用 pthread_join 等待生产者线程、计算者线程和消费者线程结束
然后再退出主程序
如果不使用 pthread_join 等待
main 函数退出主程序后，就会结束所有的线程
如果消费者还没有运行结束，也会被强制杀死
*/
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
#define ITEM_COUNT (2 * CAPACITY)

struct buffer buffer_pc, buffer_cc;

void *produce(void *arg)
{
    int i;
    int item;

    for (i = 0; i < ITEM_COUNT; i++)
    {
        item = 'a' + i;
        // printf("produce item: %c\n", item);
        buffer_put(&buffer_pc, item);
    }
    return NULL;
}

void *compute(void *arg)
{
    int i;
    int item;
    for (i = 0; i < ITEM_COUNT; i++)
    {
        item = buffer_get(&buffer_pc);
        item = toupper(item);
        // printf("compute item: %c\n",item);
        buffer_put(&buffer_cc, item);
    }
    return NULL;
}

void *consume(void *arg)
{
    int i;
    int item;
    for (i = 0; i < ITEM_COUNT; i++)
    {
        item = buffer_get(&buffer_cc);
        printf("%c\n", item);
    }
    return NULL;
}

int main()
{
    buffer_init(&buffer_pc);
    buffer_init(&buffer_cc);

    pthread_t producer_tid;
    pthread_t computer_tid;
    pthread_t consumer_tid;

    pthread_create(&producer_tid, NULL, produce, NULL);
    pthread_create(&computer_tid, NULL, compute, NULL);
    pthread_create(&consumer_tid, NULL, consume, NULL);

    pthread_join(producer_tid, NULL);
    pthread_join(computer_tid, NULL);
    pthread_join(consumer_tid, NULL);

    return 0;
}
