# 聊天系统 - 集群聊天服务器与客户端

Chat是基于muduo网络库构建的集群聊天系统，能够在NGINX TCP负载均衡环境中工作。该系统采用了Redis和MySQL作为持久化存储，同时需要NGINX的TCP负载均衡模块。以下是关于项目的主要信息以及如何编译和配置的说明。

## 功能特性

- 集群聊天服务器：实现多用户之间的即时聊天。
- 基于muduo网络库：高效的网络通信实现。
- 持久化存储：使用Redis和MySQL存储聊天记录等信息。
- 支持NGINX TCP负载均衡：可在NGINX环境中进行水平扩展，提高系统的负载能力。

## 编译方式

### 方法一：手动编译
```bash
mkdir build && mkdir bin
cd ./build
cmake ..
make
```

### 方法二：自动编译脚本
```bash
./autobuild.sh
```
