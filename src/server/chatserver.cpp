#include "chatserver.h"
#include "chatservice.h"

#include "json.hpp"
#include "utils.h"
#include <muduo/base/Logging.h>

#include <functional>
#include <string>
#include <iostream>

using json = nlohmann::json;

constexpr int MAX_PACKAGE_SIZE = 10 * 1024 * 1024;

ChatServer::ChatServer(muduo::net::EventLoop* loop, const muduo::net::InetAddress& listenAddr,
                       const muduo::string& nameArg)
    : m_server{ loop, listenAddr, nameArg }, m_loop{ loop } {
    //注册连接回调
    m_server.setConnectionCallback(std::bind(&ChatServer::onConnection, this, std::placeholders::_1));

    //注册消息回调
    m_server.setMessageCallback(
                                std::bind(&ChatServer::onMessage, this, std::placeholders::_1, std::placeholders::_2,
                                          std::placeholders::_3));

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

void ChatServer::onMessage(const muduo::net::TcpConnectionPtr& pConn, muduo::net::Buffer* pBuffer,
                           muduo::Timestamp timestamp) {
    json js;
    uint16_t messageType;
    uint32_t messageLength;

    while (pBuffer->readableBytes() >= sizeof(MessageHeader)) {
        // 读取包头，但不消耗缓冲区中的数据
        MessageHeader header;
        memcpy(&header, pBuffer->peek(), sizeof(MessageHeader));

        // 转换字节序，获取包头中各项的值
        messageType = muduo::net::sockets::networkToHost16(header.type);
        messageLength = muduo::net::sockets::networkToHost32(header.length);

        // 处理非法包
        if (messageLength <= 0 || messageLength > MAX_PACKAGE_SIZE) {
            LOG_INFO << "Illegal package, bodysize: " << header.length << ", close TcpConnection, client: " << pConn->
peerAddress().toPort();
            pConn->forceClose();

            return;
        }

        // 检查是否收到完整的包
        if (pBuffer->readableBytes() >= sizeof(MessageHeader) + messageLength) {
            // 消耗包头数据
            pBuffer->retrieve(sizeof(MessageHeader));
            LOG_INFO << "MessageHeader Received: " << sizeof(MessageHeader) << " bytes";
            LOG_INFO << "ReadableBytes: " << pBuffer->readableBytes() << " bytes";

            // 读取消息体
            std::string message = pBuffer->retrieveAsString(messageLength);
            js = json::parse(message);

            // 处理消息
            LOG_INFO << "Received message: type=" << messageType
                     << ", length=" << messageLength
                     << ", content=" << message;
        } else {
            return;
        }
    }

    auto handler{ ChatService::getInstance()->getHandler(static_cast<MsgType>(messageType)) };
    handler(pConn, js, timestamp);
}
