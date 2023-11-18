#include "connectionpool.h"
#include "mysqlconfig.h"

#include <muduo/base/Logging.h>

#include <iostream>
#include <thread>
#include <functional>

ConnectionPool* ConnectionPool::getInstance() {
    static ConnectionPool pool;
    return &pool;
}

ConnectionPool::ConnectionPool() {
    //加载配置项
    if (!loadConfigFile()) {
        LOG_INFO << __FILE__ << ":" << __LINE__ << ":" << "加载MySQL连接池配置文件失败";
    }

    //创建initSize个连接
    for (int i{}; i < m_initSize; ++i) {
        auto pConn = new Connection;
        if (pConn->connect(m_ip, m_port, m_username, m_password, m_dbname)) {
            pConn->refreshAliveTime();
            m_connectionQueue.push(pConn);
            m_connectionCount++;
        } else {
            throw std::runtime_error("MySQL连接失败！");
        }
    }

    //启动一个新的线程作为连接生产者
    std::thread producer{ [this] { produceConnTask(); }};
    producer.detach();

    //启动一个新的定时线程，扫描超过maxIdleTime空闲连接进行回收
    std::thread collector{ [this] { collectConnTask(); }};
    collector.detach();

    LOG_INFO << "MySQL连接池开启成功！";
}

bool ConnectionPool::loadConfigFile() {
    try {
        MySQLConfig config{ "/home/jmh/Chat/bin/mysql.ini" };

        m_ip = config.get("mysql", "ip");
        m_port = std::stoi(config.get("mysql", "port"));
        m_username = config.get("mysql", "username");
        m_password = config.get("mysql", "password");
        m_dbname = config.get("mysql", "dbname");

        m_initSize = std::stoi(config.get("connection_pool", "initSize"));
        m_maxSize = std::stoi(config.get("connection_pool", "maxSize"));
        m_maxIdleTime = std::stoi(config.get("connection_pool", "initIdleTime"));
        m_connectionTimeout = std::stoi(config.get("connection_pool", "connectionTimeout"));

//        m_ip = "127.0.0.1";
//        m_port = 3306;
//        m_username = "root";
//        m_password = "JMH20000527";
//        m_dbname = "chat";
//
//        m_initSize = 10;
//        m_maxSize = 1024;
//        m_maxIdleTime = 6;
//        m_connectionTimeout = 100;

        return true; // 配置文件加载成功
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;

        return false; // 配置文件加载失败
    }
}

std::shared_ptr<Connection> ConnectionPool::getConnection() {
    std::unique_lock<std::mutex> uniqueLock{ m_queueMutex };

    //队列为空
    while (m_connectionQueue.empty()) {
        //等待超时最大时间,超时醒来
        if (std::cv_status::timeout == m_cond.wait_for(uniqueLock, std::chrono::milliseconds(m_connectionTimeout))) {
            //队列依然为空
            if (m_connectionQueue.empty()) {
                LOG_INFO << __FILE__ << ":" << __LINE__ << ":" << "获取空闲连接超时，获取连接失败";
                return nullptr;
            }
        }
    }

    //队列不为空
    std::shared_ptr<Connection> pConn{ m_connectionQueue.front(), [&](Connection* p) {
        std::lock_guard<std::mutex> lockGuard{ m_queueMutex };
        pConn->refreshAliveTime();
        m_connectionQueue.push(p);
    }};

    m_connectionQueue.pop();

    //消费后，通知生产者线程检查队列是否为空
    m_cond.notify_all();

    return pConn;
}

void ConnectionPool::produceConnTask() {
    while (true) {
        std::unique_lock<std::mutex> uniqueLock{ m_queueMutex };

        //队列不空，生产线程进入等待
        while (!m_connectionQueue.empty()) {
            m_cond.wait(uniqueLock);
        }

        //生产新连接
        if (m_connectionCount < m_maxSize) {
            auto pConn = new Connection;
            pConn->connect(m_ip, m_port, m_username, m_password, m_dbname);
            pConn->refreshAliveTime();
            m_connectionQueue.push(pConn);
            m_connectionCount++;
        }

        //通知消费者线程可以消费了
        m_cond.notify_all();
    }
}

void ConnectionPool::collectConnTask() {
    while (true) {
        //睡眠maxIdleTime时间
        std::this_thread::sleep_for(std::chrono::seconds(m_maxIdleTime));

        //扫描整个队列，释放多余的连接
        std::lock_guard<std::mutex> lockGuard{ m_queueMutex };
        while (m_connectionCount > m_initSize) {
            auto* pConn{ m_connectionQueue.front() };

            if (pConn->getAliveTime() >= (m_maxIdleTime * 1000)) {
                m_connectionQueue.pop();
                m_connectionCount--;

                delete pConn;
            } else {
                break;  //队没有超时，其他连接肯定不会超时
            }
        }
    }
}