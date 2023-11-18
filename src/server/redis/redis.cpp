#include <iostream>
#include <utility>
#include "redis.h"

Redis::Redis()
    : m_publishContext{ nullptr }, m_subscribeContext{ nullptr } {
}

Redis::~Redis() {
    if (m_publishContext != nullptr) {
        redisFree(m_publishContext);
    }

    if (m_subscribeContext != nullptr) {
        redisFree(m_subscribeContext);
    }
}

bool Redis::connect() {
    // 负责publish发布消息的上下文连接
    m_publishContext = redisConnect("127.0.0.1", 6379);
    if (nullptr == m_publishContext) {
        std::cerr << "connect redis failed!" << std::endl;
        return false;
    }

    // 负责subscribe订阅消息的上下文连接
    m_subscribeContext = redisConnect("127.0.0.1", 6379);
    if (nullptr == m_subscribeContext) {
        std::cerr << "connect redis failed!" << std::endl;
        return false;
    }

    // 在单独的线程中，监听通道上的事件，有消息给业务层进行上报
    std::thread t([&]() {
        observerChannelMessage();
    });
    t.detach();

    std::cout << "connect redis-server success!" << std::endl;

    return true;
}

// 向redis指定的通道channel发布消息
bool Redis::publish(int channel, const std::string& message) {
    auto* reply = (redisReply*) redisCommand(m_publishContext, "PUBLISH %d %s", channel, message.c_str());
    if (nullptr == reply) {
        std::cerr << "publish command failed!" << std::endl;
        return false;
    }

    freeReplyObject(reply);
    return true;
}

// 向redis指定的通道subscribe订阅消息
bool Redis::subscribe(int channel) {
    // SUBSCRIBE命令本身会造成线程阻塞等待通道里面发生消息，这里只做订阅通道，不接收通道消息
    // 通道消息的接收专门在observer_channel_message函数中的独立线程中进行
    // 只负责发送命令，不阻塞接收redis server响应消息，否则和notifyMsg线程抢占响应资源
    if (REDIS_ERR == redisAppendCommand(this->m_subscribeContext, "SUBSCRIBE %d", channel)) {
        std::cerr << "subscribe command failed!" << std::endl;
        return false;
    }
    // redisBufferWrite可以循环发送缓冲区，直到缓冲区数据发送完毕（done被置为1）
    int done = 0;
    while (!done) {
        if (REDIS_ERR == redisBufferWrite(this->m_subscribeContext, &done)) {
            std::cerr << "subscribe command failed!" << std::endl;
            return false;
        }
    }
    // redisGetReply()

    return true;
}

// 向redis指定的通道unsubscribe取消订阅消息
bool Redis::unsubscribe(int channel) {
    if (REDIS_ERR == redisAppendCommand(this->m_subscribeContext, "UNSUBSCRIBE %d", channel)) {
        std::cerr << "unsubscribe command failed!" << std::endl;
        return false;
    }
    // redisBufferWrite可以循环发送缓冲区，直到缓冲区数据发送完毕（done被置为1）
    int done = 0;
    while (!done) {
        if (REDIS_ERR == redisBufferWrite(this->m_subscribeContext, &done)) {
            std::cerr << "unsubscribe command failed!" << std::endl;
            return false;
        }
    }
    return true;
}

// 在独立线程中接收订阅通道中的消息
void Redis::observerChannelMessage() {
    redisReply* reply = nullptr;
    while (REDIS_OK == redisGetReply(this->m_subscribeContext, (void**) &reply)) {
        // 订阅收到的消息是一个带三元素的数组
        if (reply != nullptr && reply->element[2] != nullptr && reply->element[2]->str != nullptr) {
            // 给业务层上报通道上发生的消息
            m_notifyMessageHandler(std::atoi(reply->element[1]->str), reply->element[2]->str);
        }

        freeReplyObject(reply);
    }

    std::cerr << ">>>>>>>>>>>>> observerChannelMessage quit <<<<<<<<<<<<<" << std::endl;
}

void Redis::initNotifyHandler(std::function<void(int, std::string)> fn) {
    this->m_notifyMessageHandler = std::move(fn);
}