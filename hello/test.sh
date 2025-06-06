# 功能
# 调用 echo 打印 hello
# 调用 echo 打印 world

SOURCE_DIR="$(pwd)/source_output"

# 检查是否存在echo文件
if [ ! -f "$SOURCE_DIR/echo" ]; then
    echo "echo文件不存在，编译echo..."
    gcc -o $SOURCE_DIR/echo echo.c
fi

cd $SOURCE_DIR

$SOURCE_DIR/echo hello
$SOURCE_DIR/echo world
