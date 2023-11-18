#ifndef CHAT_REDIS_H
#define CHAT_REDIS_H

#include <hiredis/hiredis.h>

#include <thread>
#include <functional>

class Redis {
public:
    Redis();
    ~Redis();

    // 连接redis服务器
    bool connect();

    // 向redis指定的通道channel发布消息
    bool publish(int channel, const std::string& message);

    // 向redis指定的通道subscribe订阅消息
    bool subscribe(int channel);

    // 向redis指定的通道unsubscribe取消订阅消息
    bool unsubscribe(int channel);

    // 在独立线程中接收订阅通道中的消息
    void observerChannelMessage();

    // 初始化向业务层上报通道消息的回调对象
    void initNotifyHandler(std::function<void(int, std::string)> fn);

private:
    // hiredis同步上下文对象，负责publish消息
    redisContext* m_publishContext;

    // hiredis同步上下文对象，负责subscribe消息
    redisContext* m_subscribeContext;

    // 回调操作，收到订阅的消息，给service层上报
    std::function<void(int, std::string)> m_notifyMessageHandler;
};


#endif //CHAT_REDIS_H
