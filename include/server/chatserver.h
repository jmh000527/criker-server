#ifndef CHAT_CHATSERVER_H
#define CHAT_CHATSERVER_H


#include <muduo/net/TcpServer.h>
#include <muduo/net/EventLoop.h>

//聊天服务器的主类
class ChatServer {
public:
    ChatServer(muduo::net::EventLoop* loop, const muduo::net::InetAddress& listenAddr, const muduo::string& nameArg);

    //启动服务
    void start();

private:
    void onConnection(const muduo::net::TcpConnectionPtr& pConn);
    void onMessage(const muduo::net::TcpConnectionPtr& pConn, muduo::net::Buffer* pBuffer, muduo::Timestamp timestamp);

    muduo::net::TcpServer m_server; //实现服务器功能的类对象
    muduo::net::EventLoop* m_loop;  //指向事件循环对象的指针
};


#endif //CHAT_CHATSERVER_H
