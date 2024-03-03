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
#include <openssl/bio.h>
#include <openssl/buffer.h>
#include <openssl/evp.h>

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

    std::string base64_decode(const std::string& encoded) {
        const std::string base64_chars =
            "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

        auto base64_decode_char = [&](char c) -> uint8_t {
            if (c == '=')
                return 0;
            size_t pos = base64_chars.find(c);
            if (pos != std::string::npos)
                return static_cast<uint8_t>(pos);
            else
                return 255; // Invalid character
        };

        std::string decoded;

        for (size_t i = 0; i < encoded.size(); i += 4) {
            uint32_t sextet_a = base64_decode_char(encoded[i]);
            uint32_t sextet_b = base64_decode_char(encoded[i + 1]);
            uint32_t sextet_c = base64_decode_char(encoded[i + 2]);
            uint32_t sextet_d = base64_decode_char(encoded[i + 3]);

            uint32_t triple = (sextet_a << 18) + (sextet_b << 12) + (sextet_c << 6) + sextet_d;

            decoded.push_back((triple >> 16) & 0xFF);
            if (encoded[i + 2] != '=')
                decoded.push_back((triple >> 8) & 0xFF);
            if (encoded[i + 3] != '=')
                decoded.push_back(triple & 0xFF);
        }

        return decoded;
    }

    // 自行实现的简单 Base64 编码函数
    static std::string base64_encode(const std::string& binaryData) {
        const char base64Chars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

        std::string base64String;
        size_t i = 0;

        while (i < binaryData.length()) {
            unsigned char threeBytes[3] = { 0 };
            size_t numBytes = 0;

            for (size_t j = 0; j < 3 && i < binaryData.length(); ++j) {
                threeBytes[j] = binaryData[i++];
                ++numBytes;
            }

            base64String.push_back(base64Chars[(threeBytes[0] & 0xFC) >> 2]);
            base64String.push_back(base64Chars[((threeBytes[0] & 0x03) << 4) | ((threeBytes[1] & 0xF0) >> 4)]);

            if (numBytes > 1) {
                base64String.push_back(base64Chars[((threeBytes[1] & 0x0F) << 2) | ((threeBytes[2] & 0xC0) >> 6)]);
            } else {
                base64String.push_back('=');
            }

            if (numBytes > 2) {
                base64String.push_back(base64Chars[threeBytes[2] & 0x3F]);
            } else {
                base64String.push_back('=');
            }
        }

        return base64String;
    }

    //构造消息
    std::vector<char> constructMessage(const json& js, MsgType msgtype);

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
