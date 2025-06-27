# NUAA 2025 操作系统上机考试复习

[实验课主页](https://www.nuaalab.cn/)

## 常见终端命令

```bash
# 清屏
clear # 或者Ctrl+l
# 提权
su # 提升权限到管理员
sudo command # 以管理员权限执行command
# 更改当前目录
cd /path/to/dir # . 为当前目录，.. 为上级目录
# 查看当前目录
pwd
# 查看目录下文件结构
ls # 默认为查看当前目录
  -a # 包括隐藏文件
  -l # 包括权限信息和类型信息
# 打印文件内容
cat filename # 打印filename文件中的内容到标准输出流（默认为屏幕）
# 统计
wc -l filename # 统计文件中文本行数
# 寻找特定字符串
grep [pattern] filename # 找到符合pattern的字符串
# 文件操作
mkdir path/to/dir # 创建路径
touch filename # 创建文件
chmod # 权限控制
	+x # 添加可执行权限
rm filename# 删除
	-r # 递归删除
	-f # 强制删除
mv /old/path/to/filename1 /new/path/to/filename2 # 重命名/移动文件、文件夹
cp /old/path/to/filename1 /new/path/to/filename2 # 复制
	-r # 递归复制
# 解压 .tgz文件
tar zxvf file.tgz
# 打印字符串到终端
echo "string" # 打印字符串string到标准输出流

# 输出/输入重定向
echo 111 > filename # 将echo要打印的111字符串输出重定向到filename文件中
echo 111 >> filename # 同上，模式从覆盖改为追加
cat < /etc/passwd # 将/etc/passwd文件内容作为cat命令的参数
# 管道
command1 | command2 # 创建管道，将command1的输出作为command2的输入
cat < /etc/passwd | wc -l > result.txt # 打印/etc/passwd中的内容作为wc的参数，统计内容的行数，将结果输出到result.txt文件中
# 即统计/etc/passwd文件的行数，将结果保存到result.txt文件中
```

## vim基础操作

### vim的三种编辑状态

1. NORMAL / COMMAND：按`esc`进入命令模式
2. VISUAL：按`v`进入选择模式
3. INSERT：按`i`进入插入模式

### 常用快捷键

```bash
NORMAL：
  dd # 删除整行
  gg # 回到文档开头
  GG # 跳到文档末尾
  /<pattern> # 查找符合pattern的字符
  	n # 下一个
  	N # 上一个
  u # 撤销上一步操作
  U # 撤销当前行的所有修改
  Ctrl + r # 恢复撤销操作
VISUAL：
  y # 复制
  p # 粘贴
  c # 剪切
  d # 删除
光标移动(NORMAL)：
  h # 左
  j # 下
  k # 上
  l # 右
  0 # 行首
  $ # 行尾
  w # 移动到下一个单词头部
  b # 移动到前一个单词头部
  e # 移动到下一个单词尾部
  ge # 移动到前一个单词尾部
```

### vim操作

```bash
# 编辑文件
vim filename
# 退出文件，先进入命令状态
:wq # 保存并退出
:q  # 未进行改动后退出
:q! # 强制退出，不保存
```

## 怎么看Makefile

```makefile
CC           := gcc # 编译器
LD           := gcc # 链接器
# 编译选项
CFLAGS       += -Wall
CFLAGS       += -fno-common
CFLAGS 	     += -g
CFLAGS       += -fsanitize=address
# 命令(make + command)
all:  # make all
	cc $(CFLAGS) -o T1 T1.c # 编译T1.c至T1
	cc $(CFLAGS) -o T2 T2.c # 编译T2.c至T2

clean: # make clean
	rm -f T1 T2 # 清理多余文件
```

## 常用API

```c
/*
 * fd 是一个长度为2的整型数组，fd[0] 表示管道的读端，fd[1] 表示管道的写端。
 * 调用成功返回0，失败返回-1。
 * 子进程中pid为0
**/
int pipe(int fd[2]);

/*
 * 用于文件描述符重定向的系统调用。
 * oldfd：原有的文件描述符。
 * newfd：希望复制到的新文件描述符。
 * dup2 会将 newfd 变成 oldfd 的副本。之后对 newfd 的读写操作，实际上就是对 oldfd 的操作。如果 newfd 已经打开，会先自动关闭它。常用于重定向标准输入（0）、标准输出（1）、标准错误（2）等。
**/
int dup2(int oldfd, int newfd);

/*
 * 调用一次，返回两次：在父进程中返回子进程的PID，在子进程中返回0。如果出错，返回-1。
 * 父子进程从 fork 之后的下一条语句开始分别执行，拥有独立的地址空间。
**/
pid_t fork(void);

/*
 * 调用系统命令程序
 * file：要执行的程序名（会在 PATH 环境变量指定的目录中查找）。
 * arg0：新程序的名称（通常与 file 相同）。后面可以跟多个参数，最后必须以 NULL 结尾。
**/
int execlp(const char *file, const char *arg0, ..., NULL);

/*
 * thread：指向 pthread_t 类型的变量，用于存放新线程的ID。
 * attr：线程属性，通常用 NULL 表示默认属性。
 * start_routine：线程将要执行的函数指针，函数原型为 void *func(void *)。
 * arg：传递给线程函数的参数，可以为 NULL。
 * 返回值为0表示创建成功，非0表示出错。
**/
int pthread_create(pthread_t *thread, const pthread_attr_t *attr,
                   void *(*start_routine)(void *), void *arg);

/*
 * thread：要等待的线程ID。
 * retval：用于获取线程的返回值，若不需要可传 NULL。
 * 调用 pthread_join 后，当前线程会阻塞，直到被等待的线程终止。常用于主线程等待所有子线程完成后再退出，保证多线程程序的正确性和资源回收。
**/
int pthread_join(pthread_t thread, void **retval);
```

## 大致考试范围

### 进程、进程间通讯

- `fork`  和 `pipe` 的使用
- 重定向操作

### 进程共享资源控制

- mutex信号量操作

```c
// 初始化互斥锁
pthread_mutex_init(&mutex, NULL);
// 加锁（上锁）
pthread_mutex_lock(&mutex);
// 解锁（释放锁）
pthread_mutex_unlock(&mutex);
// 销毁互斥锁
pthread_mutex_destroy(&mutex);
// 唤醒等待某个条件变量的一个线程
int pthread_cond_signal(pthread_cond_t *cond);
// 用于让线程等待某个条件变量的变化
int pthread_cond_wait(pthread_cond_t *cond, pthread_mutex_t *mutex);
```

- 进程间通讯、线程间通讯

### 生产者、计算者、消费者机制

**单一生产者-消费者**

生产者、消费者共享一个初始为空、大小为n的缓冲区。只有缓冲区没满时，生产者才能把产品放入缓冲区，否则必须等待。只有缓冲区不空时，消费者才能从中取出产品，否则必须等待。缓冲区是临界资源，各进程必须互斥地访问。

![单一生产者-消费者](https://i-blog.csdnimg.cn/blog_migrate/d353e64f6004cc26916aad3f7b013782.png)

**多生产者-多消费者**

桌子上有一只盘子，每次只能向其中放入一个水果。爸爸专向盘子中放苹果，妈妈专向盘子中放橘子，儿子专等着吃盘子中的橘子，女儿专等着吃盘子中的苹果。只有盘子空时，爸爸或妈妈才可向盘子中放一个水果。仅当盘子中有自己需要的水果时，儿子或女儿可以从盘子中取出水果。

![多生产者-多消费者](https://i-blog.csdnimg.cn/blog_migrate/a586db8fa5f51fcaf803dc98a6adca0d.png)

### 文件系统

- [课件](https://www.nuaalab.cn/os/fs)
- fuse框架
- uxfs文件系统

```c
// 索引节点
// 从 inode 中偏移为 position 的位置读取数据，保存到起始位置为 memory、大小为 size 的内存区域
int inode_read(inode_t *this, off_t position, void *memory, int size);
// 把起始位置为 memory、大小为 size 的内存区域，写入到 inode 中偏移为 position 的位置
int inode_write(inode_t *this, off_t position, void *memory, int size);

// 示例：向mem中读入"bcd"
inode content: abcdef
  			   012345
pos = 1;
char mem[3];
cnt = inode_read(inode, pos, mem, sizeof(mem));

// 目录项
// 目录项结构体
typedef struct dir_entry {
    char name[28]; // 名称
    int ino; // 索引节点号
} dir_entry_t;
// 目录游标（了解）
typedef struct {
    inode_t *inode;
    int offset;
    void *buff;
} dir_cursor_t;
/*
 * dir cursor 是一个用于遍历目录中的游标
 * dir_cursor_next
 * 	返回当前指向的目录项
 *	如果返回 NULL，表示遍历结束
 *	让游标指向下一个目录项
*/
void dir_cursor_init(dir_cursor_t *this, inode_t *inode);
dir_entry_t *dir_cursor_next(dir_cursor_t *this);
void dir_cursor_destroy(dir_cursor_t *this);
// 遍历目录 inode 的每一个目录项，打印 name 和 ino
void dir_visit(inode_t *this)
{
    dir_entry_t *dir_entry;
    dir_cursor_t dir_cursor;

    dir_cursor_init(&dir_cursor, this);

    while (dir_entry = dir_cursor_next(&dir_cursor)) {
        printf("name = %s\n", dir_entry->name)
        printf(" ino = %s\n", dir_entry->ino)
    }

    dir_cursor_destroy(&dir_cursor); //释放游标
}

// namei（掌握open）
int namei_open(const char *path, inode_t **ip); // 返回找到的inode节点，若失败则函数返回-1
// 示例
char *path = "/A/C";
inode_t *p;
int error = namei_open(path, &p);
```

- 实现对文件系统中保存的文件的读

### 多线程开发

```c
// 初始化三个线程id
pthread_t producer_tid;
pthread_t computer_tid;
pthread_t consumer_tid;
// 绑定线程所要执行的任务，获取线程id
pthread_create(&producer_tid, NULL, produce, NULL);
pthread_create(&computer_tid, NULL, compute, NULL);
pthread_create(&consumer_tid, NULL, consume, NULL);
// 等待线程执行结束
pthread_join(producer_tid, NULL);
pthread_join(computer_tid, NULL);
pthread_join(consumer_tid, NULL);
```

## 经验（？）
在考试环境里面做了去年的题目，浅浅总结一下心得。

### 1. 快速定位需要编辑的位置
因为题目为挖空完成TODO的形式，所以可以利用一些命令定位要修改、补全的代码位置。

```bash
# 利用grep搜索TODO标签，定位需要编辑的代码文件
grep TODO -r .
# vim编辑文件时，利用查找功能快速定位
/TODO # 借助n、N快速查找下一、上一挖空的位置
```

### 2. 代码编辑器分屏
在编辑多文件得题目时，可能需要在多个文件中寻找一些变量、函数的定义。
可以借助vim的分屏功能，同时打开两个甚至多个文件编辑区域。

```bash
# 水平方向分屏打开新文件（上下分屏）
:sp file2.c
# 垂直方向分屏打开新文件（左右分屏）
:sp file2.c
# 也可以在打开时分屏
vim -on file1, file2 ... # 上下分屏 
vim -On file1, file2 ... # 左右分屏
# 切换当前编辑区域 Ctrl+w进入窗口管理模式
Ctrl+w h j k l
Ctrl+w 上下左右方向键
```

### 3. 批量替换
当出现拼写错误时，可以利用全局替换功能进行批量替换。

```bash
:%s/oldpattern/newstring/
```

有些结构重复的代码片段可以用复制粘贴组合键来快速填充代码框架。
总之就是用一些快捷键和常见命令来替代只按上下左右键查看代码的慢操作。 ~~这就去学vim骚操作~~

----
个人整理，有误请帮忙指出。
