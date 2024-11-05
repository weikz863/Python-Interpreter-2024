#!/bin/bash

# 在脚本中显式定义 grun alias（如果你仍想使用 alias）
alias grun='java -Xmx500M -cp "/usr/local/lib/antlr-4.13.1-complete.jar:$CLASSPATH" org.antlr.v4.gui.TestRig'

# 检查 antlr4 命令是否存在
if ! command -v antlr4 &> /dev/null; then
    echo "Error: antlr4 command not found."
    exit 1
fi

# 检查 Python3Demo.g4 文件是否存在
if [ ! -f "Python3Demo.g4" ]; then
    echo "Error: Python3Demo.g4 not found."
    exit 1
fi

# 设置工作目录
output_dir="/tmp/Python3Demo"

# 如果输出目录不存在，则创建它
if [ ! -d "$output_dir" ]; then
    mkdir -p "$output_dir"
fi

# 如果目录中没有 Java 文件或文件不存在，才执行 antlr4
if [ ! -f "$output_dir/Python3Demo*.java" ]; then
    echo "Running antlr4 to generate Java files..."
    antlr4 -o "$output_dir" Python3Demo.g4
fi

# 如果 Java 文件不存在，或者 .class 文件不存在，则运行 javac
if [ ! -f "$output_dir/Python3Demo.class" ]; then
    echo "Compiling Java files..."
    javac "$output_dir"/Python3Demo*.java
fi

# 切换到输出目录，然后运行 grun（即 java）命令，并将所有参数传递给它
echo "Running visualization with arguments: $@"
cd "$output_dir" && java -Xmx500M -cp "/usr/local/lib/antlr-4.13.1-complete.jar:$CLASSPATH" org.antlr.v4.gui.TestRig Python3Demo "$@"
