#ifndef CHAT_MSGTYPE_H
#define CHAT_MSGTYPE_H

//server和client的公共文件

#include <string>

// 定义包头结构
#pragma pack(push, 1)  // 关闭内存对齐
struct MessageHeader {
    uint16_t type;   // 消息类型
    uint32_t length; // 消息长度
};
#pragma pack(pop)  // 恢复默认的内存对齐方式

enum class MsgType {
    LOGIN_MSG = 1,
    //登陆消息
    REG_MSG,
    //注册消息
    LOGIN_MSG_ACK,
    //登陆响应消息
    REG_MSG_ACK,
    //注册响应消息
    ONE_CHAT_MSG,
    //聊天消息
    ADD_FRIEND_MSG,
    //添加好友消息

    CREATE_GROUP_MSG,
    //创建群组消息
    ADD_GROUP_MSG,
    //加入群组消息
    GROUP_CHAT_MSG,
    //群聊天消息

    LOGOUT_MSG,
    //注销登陆消息
};

// 自定义to_string函数
static std::string enum_to_string(MsgType msgType) {
    switch (msgType) {
        case MsgType::LOGIN_MSG:
            return "LOGIN_MSG";
        case MsgType::REG_MSG:
            return "REG_MSG";
        case MsgType::REG_MSG_ACK:
            return "REG_MSG_ACK";
        case MsgType::LOGIN_MSG_ACK:
            return "LOGIN_MSG_ACK";
        default:
            return "Invalid MsgType";
    }
}

#endif //CHAT_MSGTYPE_H
