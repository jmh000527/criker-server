# Cirker即时通讯软件集群服务器

Criker服务端是于muduo网络库构建，能够在NGINX TCP负载均衡环境中工作。该系统采用了Redis发布-订阅模式作集群化和MySQL作为数据库存储，同时可搭配使用NGINX的TCP负载均衡模块。以下是关于项目的主要信息以及如何编译和配置的说明。

## 功能特性

- 集群聊天服务器：实现多用户之间的即时聊天。使用Redis发布-订阅模式进行集群化。
- 基于muduo网络库：高效的网络通信实现。
- 持久化存储：使用MySQL存储聊天记录等信息。
- 支持NGINX TCP负载均衡：可在NGINX环境中进行水平扩展，提高系统的负载能力。
- 使用MySQL连接池管理并发MySQL连接。

## 配置文件
MySQL连接池配置文件mysql.ini放置于bin目录中，运行时请保证该配置文件与编译产生的可执行文件处于同一目录下。

## 编译方式
请首先确保您使用的g++编译器支持C++20标准。

### 方法一：手动编译
```bash
mkdir build
cd ./build
cmake ..
make
```

### 方法二：自动编译脚本
```bash
./autobuild.sh
```
