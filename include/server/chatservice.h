#ifndef CHAT_CHATSERVICE_H
#define CHAT_CHATSERVICE_H


#include "msgtype.h"

#include "usermodel.h"
#include "offlinemessagemodel.h"
#include "friendmodel.h"
#include "groupmodel.h"
#include "redis.h"

#include "json.hpp"
#include <muduo/net/TcpConnection.h>

#include <unordered_map>
#include <functional>
#include <memory>
#include <mutex>
#include <vector>

using json = nlohmann::json;

//处理消息的事件回调方法类型
using MsgHandler = std::function<void(const muduo::net::TcpConnectionPtr&, json&, muduo::Timestamp)>;

//聊天服务器业务类
class ChatService {
public:
    //获取单例静态对象
    static ChatService* getInstance();

    //处理登陆业务
    void login(const muduo::net::TcpConnectionPtr& pConn, json& js, muduo::Timestamp timestamp);

    //处理注册业务
    void reg(const muduo::net::TcpConnectionPtr& pConn, json& js, muduo::Timestamp timestamp);

    //一对一聊天业务
    void oneChat(const muduo::net::TcpConnectionPtr& pConn, json& js, muduo::Timestamp timestamp);

    //添加好友业务
    void addFriend(const muduo::net::TcpConnectionPtr& pConn, json& js, muduo::Timestamp timestamp);

    //创建群组业务
    void createGroup(const muduo::net::TcpConnectionPtr& pConn, json& js, muduo::Timestamp timestamp);

    //加入群组业务
    void addGroup(const muduo::net::TcpConnectionPtr& pConn, json& js, muduo::Timestamp timestamp);

    //群组聊天业务
    void groupChat(const muduo::net::TcpConnectionPtr& pConn, json& js, muduo::Timestamp timestamp);

    //退出登录业务
    void logout(const muduo::net::TcpConnectionPtr& pConn, json& js, muduo::Timestamp timestamp);

    //处理客户端异常退出
    void clientCloseException(const muduo::net::TcpConnectionPtr& pConn);

    //处理服务器SIGINT退出的重置方法
    void reset();

    //获取消息对应的Handler
    MsgHandler getHandler(MsgType msgType);

    //从redis消息队列中获取订阅的消息
    void handleRedisSubscribeMessage(int userid, const std::string& msg);

private:
    ChatService();

    //存储消息ID及其处理器
    std::unordered_map<MsgType, MsgHandler> m_msgHandlerMap;

    //存储在线用户的通讯连接
    std::unordered_map<int, muduo::net::TcpConnectionPtr> m_userConnMap;

    //用于保证m_userConnMap线程安全的互斥锁
    std::mutex m_connMutex;

    //数据操作类对象
    UserModel m_userModel;
    OfflineMessageModel m_offlineMessageModel;
    FriendModel m_friendModel;
    GroupModel m_groupModel;

    //Redis操作对象
    Redis m_redis;
};


#endif //CHAT_CHATSERVICE_H
