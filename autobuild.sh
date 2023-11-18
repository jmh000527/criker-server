#!/bin/bash

# 清理之前的构建
rm -rf build
mkdir build
cd build

# 运行 CMake
cmake ..

# 使用 make 构建项目
make

# 返回到项目根目录
cd ..
