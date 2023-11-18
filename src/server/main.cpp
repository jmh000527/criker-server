#include "chatserver.h"
#include "chatservice.h"

#include <csignal>

#include <iostream>

//处理服务器SIGINT（Ctrl + C）结束后重置user的状态信息
void resetHandler(int) {
    ChatService::getInstance()->reset();
    exit(EXIT_SUCCESS);
}

int main(int argc, char** argv) {
    if (argc < 3) {
        std::cerr << "command invalid! example: ./ChatServer 127.0.0.1 6000" << std::endl;
        exit(-1);
    }

    // 解析通过命令行参数传递的ip和port
    char* ip = argv[1];
    uint16_t port = atoi(argv[2]);


    std::signal(SIGINT, &resetHandler);

    muduo::net::EventLoop loop;
    muduo::net::InetAddress addr{ ip, port };
    ChatServer server{ &loop, addr, "ChatServer" };

    server.start();
    loop.loop();
}