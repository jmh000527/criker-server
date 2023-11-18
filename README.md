# chat
可在NGINX TCP负载均衡环境中工作的集群聊天服务器和客户端，基于muduo实现，使用了Redis，MySQL，NGINX等。

编译方式
mkdir build && mkdir bin
cd ./build
cmake ..
make

需要NGINX TCP负载均衡模块——从源码编译时--with-stream
