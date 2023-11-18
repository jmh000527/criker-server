#include "chatserver.h"
#include "chatservice.h"

#include "json.hpp"
#include "utils.h"
#include <muduo/base/Logging.h>

#include <functional>
#include <string>
#include <iostream>

using json = nlohmann::json;

ChatServer::ChatServer(muduo::net::EventLoop* loop, const muduo::net::InetAddress& listenAddr,
                       const muduo::string& nameArg)
    : m_server{ loop, listenAddr, nameArg }, m_loop{ loop } {
    //注册连接回调
    m_server.setConnectionCallback(std::bind(&ChatServer::onConnection, this, std::placeholders::_1));

    //注册消息回调
    m_server.setMessageCallback(
        std::bind(&ChatServer::onMessage, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));

    //设置线程数量
    m_server.setThreadNum(4);
}

void ChatServer::start() {
    m_server.start();
}

void ChatServer::onConnection(const muduo::net::TcpConnectionPtr& pConn) {
    //连接失败，用户断开连接
    if (!pConn->connected()) {
        LOG_INFO << "用户断开连接！";

        ChatService::getInstance()->clientCloseException(pConn);
        pConn->shutdown();
    }
}

void ChatServer::onMessage(const muduo::net::TcpConnectionPtr& pConn, muduo::net::Buffer* buffer,
                           muduo::Timestamp timestamp) {
    //获取数据
    std::string buf{ buffer->retrieveAllAsString() };

    //测试json
    std::cout << buf << std::endl;

    //数据反序列化
    json js{ json::parse(buf) };

    auto msgType = static_cast<MsgType>(getValueFromJson<int>(js, "msgtype"));
    auto handler{ ChatService::getInstance()->getHandler(msgType) };

    handler(pConn, js, timestamp);
}
