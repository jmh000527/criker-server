#ifndef CHAT_CONNECTIONPOOL_H
#define CHAT_CONNECTIONPOOL_H


#include "connection.h"

#include <queue>
#include <mutex>
#include <atomic>
#include <memory>
#include <condition_variable>

class ConnectionPool {
public:
    static ConnectionPool* getInstance();

    //获取一个可用空闲连接，消费者
    std::shared_ptr<Connection> getConnection();

private:
    ConnectionPool();

    //从配置文件加载配置项
    bool loadConfigFile();

    //运行在独立线程中，专门负责生产新连接
    void produceConnTask();

    //运行在独立线程中，专门空闲连接回收
    void collectConnTask();

    std::string m_ip;
    unsigned short m_port;
    std::string m_username;
    std::string m_password;
    std::string m_dbname;

    size_t m_initSize;     //初始连接量
    size_t m_maxSize;      //最大连接量
    int m_maxIdleTime;  //最大空闲时间
    int m_connectionTimeout;    //获取连接的超时时间

    std::queue<Connection*> m_connectionQueue;  //连接存储队列
    std::mutex m_queueMutex;    //维护连接队列线程安全的互斥锁
    std::atomic<int> m_connectionCount; //记录创建的连接总数量
    std::condition_variable m_cond; //用于生产者线程和消费者线程的通信
};


#endif //CHAT_CONNECTIONPOOL_H
