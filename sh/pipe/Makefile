# 编译器和编译选项
CC 		= gcc
CFLAGS 	= -Wall -Wextra -std=c99

# 目标可执行文件
TARGET 	= sh

# 源文件
SOURCES = main.c parse.c cmd.c

# 头文件
HEADERS = parse.h cmd.h

# 目标文件
OBJECTS = $(SOURCES:.c=.o)

# 默认目标
all: $(TARGET)

# 生成可执行文件
$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $^

# 生成目标文件的规则
%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

# 清理生成的文件
clean:
	rm -f $(OBJECTS) $(TARGET)
	rm -f output.txt

# 重新编译
rebuild: clean all

# 运行程序
run: $(TARGET)
	./$(TARGET)

# 声明伪目标
.PHONY: all clean rebuild run


