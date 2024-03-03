#include "chatservice.h"
#include "utils.h"

#include <muduo/base/Logging.h>
#include <string>
#include <iostream>
#include <map>

ChatService* ChatService::getInstance() {
    static ChatService service;
    return &service;
}

void ChatService::login(const muduo::net::TcpConnectionPtr& pConn, json& js, muduo::Timestamp timestamp) {
    auto id{ getValueFromJson<int>(js, "id") };
    auto password{ getValueFromJson<std::string>(js, "password") };

    User user{ m_userModel.query(id) };
    if (user.getId() == id && user.getPassword() == password) {
        if (user.getState() == "online") {
            LOG_INFO << "该用户已登录，请勿重复登陆！";

            json response;
            response["msgtype"] = static_cast<int>(MsgType::LOGIN_MSG_ACK);
            response["errno"] = 2;
            response["errmsg"] = "this account is using, try another.";

            //构造消息
            std::vector<char> messageData = constructMessage(response, MsgType::LOGIN_MSG_ACK);

            pConn->send(messageData.data(), messageData.size());
        } else {
            //登录成功

            {
                //记录用户连接信息
                std::lock_guard<std::mutex> lockGuard{ m_connMutex };
                m_userConnMap.insert({ user.getId(), pConn });
            }

            //向Redis订阅channel(userid)
            m_redis.subscribe(user.getId());

            //更新用户状态信息
            user.setState("online");
            m_userModel.updateState(user);

            json response;
            response["msgtype"] = static_cast<int>(MsgType::LOGIN_MSG_ACK);
            response["errno"] = 0;
            response["id"] = user.getId();
            response["name"] = user.getName();
            response["headimage"] = user.getHeadImage();

            //查询该用户是否有离线消息
            auto offlineMsgs{ m_offlineMessageModel.query(user.getId()) };
            if (!offlineMsgs.empty()) {
                //从OfflineMessage结构中取出消息存放到一个vector中赋值给json
                std::vector<std::string> msgs{};
                for (const auto& offlineMsg: offlineMsgs) {
                    msgs.push_back(offlineMsg.getMessage());
                }

                //向响应中添加离线消息
                response["offlinemsg"] = msgs;

                //清空该用户的离线消息
                m_offlineMessageModel.remove(user.getId());
            }

            //查询该用户的好友信息
            auto friends{ m_friendModel.query(user.getId()) };
            if (!friends.empty()) {
                std::vector<std::string> friendsInfo{};
                for (const auto& friendUser: friends) {
                    json js;
                    js["id"] = friendUser.getId();
                    js["name"] = friendUser.getName();
                    js["state"] = friendUser.getState();
                    js["headimage"] = friendUser.getHeadImage();

                    friendsInfo.push_back(js.dump());
                }

                //向响应中添加当前账户好友信息
                response["friends"] = friendsInfo;
            }

            //查询用户加入的群组
            std::vector<Group> groupsQuery{ m_groupModel.queryGroups(user.getId()) };
            if (!groupsQuery.empty()) {
                std::vector<std::string> groups; //保存所有用户加入的群组，及每个群组用户的信息
                for (auto& group: groupsQuery) {
                    json groupJson;
                    groupJson["id"] = group.getId();
                    groupJson["groupname"] = group.getName();
                    groupJson["groupdesc"] = group.getDesc();

                    std::vector<std::string> groupUsers; //保存每个群组中的所有群组用户信息
                    for (auto& groupUser: group.getUsers()) {
                        json js;
                        js["id"] = groupUser.getId();
                        js["name"] = groupUser.getName();
                        js["state"] = groupUser.getState();
                        js["role"] = groupUser.getRole();
                        js["headimage"] = groupUser.getHeadImage();

                        groupUsers.push_back(js.dump());
                    }

                    groupJson["users"] = groupUsers;
                    groups.push_back(groupJson.dump());
                }

                response["groups"] = groups;
            }

            //构造消息
            std::vector<char> messageData = constructMessage(response, MsgType::LOGIN_MSG_ACK);

            //发送登陆成功确认，LOGIN_MSG_ACK
            pConn->send(messageData.data(), messageData.size());

            LOG_INFO << "用户：" << user.getName() << " ID：" << user.getId() << " 登陆成功！";
        }
    } else {
        LOG_INFO << "用户不存在或密码错误，登陆失败！";

        json response;
        response["msgtype"] = static_cast<int>(MsgType::LOGIN_MSG_ACK);
        response["errno"] = 1;
        response["errmsg"] = "invalid id or password!";

        //构造消息
        std::vector<char> messageData = constructMessage(response, MsgType::LOGIN_MSG_ACK);

        pConn->send(messageData.data(), messageData.size());
    }
}

void ChatService::reg(const muduo::net::TcpConnectionPtr& pConn, json& js, muduo::Timestamp timestamp) {
    auto name{ getValueFromJson<std::string>(js, "name") };
    auto password{ getValueFromJson<std::string>(js, "password") };

    std::string base64Image = js["headimage"];

    // 将 Base64 解码为二进制数据
    std::string imageBinary = base64_decode(base64Image);

    User user{ imageBinary, -1, name, password, "offline" };
    if (bool res = m_userModel.insert(user); res) {
        LOG_INFO << "注册成功！";

        json response;
        response["msgtype"] = static_cast<int>(MsgType::REG_MSG_ACK);
        response["errno"] = 0;
        response["id"] = user.getId();

        //构造消息
        std::vector<char> messageData = constructMessage(response, MsgType::REG_MSG_ACK);
        pConn->send(messageData.data(), messageData.size());
    } else {
        LOG_INFO << "注册失败！";

        json response;
        response["msgtype"] = static_cast<int>(MsgType::REG_MSG_ACK);
        response["errno"] = 1;

        //构造消息
        std::vector<char> messageData = constructMessage(response, MsgType::REG_MSG_ACK);
        pConn->send(messageData.data(), messageData.size());
    }
}

std::vector<char> ChatService::constructMessage(const json& js, MsgType msgtype) {
    // 构造包头
    MessageHeader header;
    header.type = htons(static_cast<uint16_t>(msgtype)); // 你可以定义不同的消息类型
    header.length = htonl(static_cast<uint32_t>(js.dump().size()));

    // 合并包头和消息为一个连续的字节数组
    std::vector<char> messageData(sizeof(header) + js.dump().size());

    std::memcpy(messageData.data(), &header, sizeof(header));
    std::memcpy(messageData.data() + sizeof(header), js.dump().c_str(), js.dump().size());

    auto str = js.dump();

    LOG_INFO << "SentBytes: " << js.dump().size() << " bytes";

    return messageData;
}

void ChatService::oneChat(const muduo::net::TcpConnectionPtr& pConn, json& js, muduo::Timestamp timestamp) {
    auto toId{ getValueFromJson<int>(js, "toid") };

    bool userOnline{ false }; {
        std::lock_guard<std::mutex> lockGuard{ m_connMutex };
        auto iter{ m_userConnMap.find(toId) };
        if (iter != std::end(m_userConnMap)) {
            //ID为toId的用户在线
            userOnline = true;

            std::vector<char> messageData = constructMessage(js, MsgType::ONE_CHAT_MSG);

            // 发送整体消息
            iter->second->send(messageData.data(), messageData.size());

            return;
        }
    }

    // 查询toid是否在线,区分是否在其他服务器上登陆
    User user{ m_userModel.query(toId) };
    if (user.getState() == "online") {
        // std::vector<char> messageData = constructMessage(js, MsgType::ONE_CHAT_MSG);
        m_redis.publish(toId, js.dump());
    } else {
        //ID为toId的用户不在线，也没有登陆在其他服务器上，存储离线消息
        m_offlineMessageModel.insert(OfflineMessage{ toId, js.dump() });
    }
}

void ChatService::addFriend(const muduo::net::TcpConnectionPtr& pConn, json& js, muduo::Timestamp timestamp) {
    auto userid{ getValueFromJson<int>(js, "id") };
    auto friendid{ getValueFromJson<int>(js, "friendid") };

    //存储好友信息
    m_friendModel.insert(Friend{ userid, friendid });
}

void ChatService::createGroup(const muduo::net::TcpConnectionPtr& pConn, json& js, muduo::Timestamp timestamp) {
    auto userid{ getValueFromJson<int>(js, "id") };
    auto groupname{ getValueFromJson<std::string>(js, "groupname") };
    auto groupdesc{ getValueFromJson<std::string>(js, "groupdesc") };

    //存储新创建的群组
    Group group{ -1, groupname, groupdesc };
    if (m_groupModel.createGroup(group)) {
        //存储群组创建人信息
        m_groupModel.addGroup(userid, group.getId(), "creator");
    }
}

void ChatService::addGroup(const muduo::net::TcpConnectionPtr& pConn, json& js, muduo::Timestamp timestamp) {
    auto userid{ getValueFromJson<int>(js, "id") };
    auto groupid{ getValueFromJson<int>(js, "groupid") };

    m_groupModel.addGroup(userid, groupid, "normal");
}

void ChatService::groupChat(const muduo::net::TcpConnectionPtr& pConn, json& js, muduo::Timestamp timestamp) {
    auto userid{ getValueFromJson<int>(js, "id") };
    auto groupid{ getValueFromJson<int>(js, "groupid") };

    auto groupUserIdVec{ m_groupModel.queryGroupUsers(userid, groupid) };

    std::lock_guard<std::mutex> lockGuard{ m_connMutex };
    for (auto id: groupUserIdVec) {
        auto iter{ m_userConnMap.find(id) };

        if (iter != std::end(m_userConnMap)) {
            //当前群用户在线，转发群消息
            std::vector<char> messageData = constructMessage(js, MsgType::GROUP_CHAT_MSG);

            // 发送整体消息
            iter->second->send(messageData.data(), messageData.size());
            // iter->second->send(js.dump());
        } else {
            // 查询toid是否在线，是否在其他服务器上登陆
            User user{ m_userModel.query(id) };
            if (user.getState() == "online") {
                m_redis.publish(id, js.dump());
            } else {
                // 存储离线群消息
                m_offlineMessageModel.insert(OfflineMessage{ id, js.dump() });
            }
        }
    }
}

void ChatService::logout(const muduo::net::TcpConnectionPtr& pConn, json& js, muduo::Timestamp timestamp) {
    auto userid{ getValueFromJson<int>(js, "id") };
    auto name{ getValueFromJson<std::string>(js, "name") }; {
        std::lock_guard<std::mutex> lockGuard(m_connMutex);
        auto iter = m_userConnMap.find(userid);
        if (iter != std::end(m_userConnMap)) {
            m_userConnMap.erase(iter);
        }
    }

    // 用户注销，相当于就是下线，在redis中取消订阅通道
    m_redis.unsubscribe(userid);

    // 更新用户的状态信息
    User user(userid, name, "", "offline");
    m_userModel.updateState(user);

    LOG_INFO << "用户：" << user.getName() << " ID：" << user.getId() << " 注销登录！";
}

void ChatService::clientCloseException(const muduo::net::TcpConnectionPtr& pConn) {
    User user; {
        std::lock_guard<std::mutex> lockGuard{ m_connMutex };
        for (auto [key, value]: m_userConnMap) {
            if (value == pConn) {
                //从m_userConnMap中删除用户连接信息
                user.setId(key);
                m_userConnMap.erase(key);
                break;
            }
        }
    }

    // 用户注销，相当于就是下线，在redis中取消订阅通道
    m_redis.unsubscribe(user.getId());

    //存在有效用户，更新用户的状态信息
    if (user.getId() != -1) {
        user.setState("offline");
        m_userModel.updateState(user);
    }
}

void ChatService::reset() {
    //将所有online状态的用过户设置为offline
    m_userModel.resetState();
}

MsgHandler ChatService::getHandler(MsgType msgType) {
    auto iter{ m_msgHandlerMap.find(msgType) };

    //没有注册该消息类型的Handler
    if (iter == std::end(m_msgHandlerMap)) {
        //返回一个默认Handler，什么也不做
        return [msgType](auto& pConn, auto& js, auto timestamp) {
            LOG_ERROR << "msgType: " << enum_to_string(msgType) << " cannot find a handler!";
        };
    } else {
        return m_msgHandlerMap[msgType];
    }
}

void ChatService::handleRedisSubscribeMessage(int userid, const std::string& msg) {
    std::lock_guard<std::mutex> lock{ m_connMutex };
    auto iter = m_userConnMap.find(userid);
    if (iter != std::end(m_userConnMap)) {
        //构造消息
        std::vector<char> messageData = constructMessage(json::parse(msg),
                                                         getValueFromJson<MsgType>(json::parse(msg), "msgtype"));

        iter->second->send(messageData.data(), messageData.size());
        return;
    }

    // 存储该用户的离线消息，防止在Redis消息队列中转途中，用户下线了
    m_offlineMessageModel.insert(OfflineMessage{ userid, msg });
}

ChatService::ChatService() {
    //注册消息以及对应Handler
    m_msgHandlerMap.insert({ MsgType::LOGIN_MSG,
                             std::bind(&ChatService::login, this, std::placeholders::_1, std::placeholders::_2,
                                       std::placeholders::_3) });
    m_msgHandlerMap.insert({ MsgType::REG_MSG,
                             std::bind(&ChatService::reg, this, std::placeholders::_1, std::placeholders::_2,
                                       std::placeholders::_3) });
    m_msgHandlerMap.insert({ MsgType::ONE_CHAT_MSG,
                             std::bind(&ChatService::oneChat, this, std::placeholders::_1, std::placeholders::_2,
                                       std::placeholders::_3) });
    m_msgHandlerMap.insert({ MsgType::ADD_FRIEND_MSG,
                             std::bind(&ChatService::addFriend, this, std::placeholders::_1, std::placeholders::_2,
                                       std::placeholders::_3) });
    m_msgHandlerMap.insert({ MsgType::CREATE_GROUP_MSG,
                             std::bind(&ChatService::createGroup, this, std::placeholders::_1, std::placeholders::_2,
                                       std::placeholders::_3) });
    m_msgHandlerMap.insert({ MsgType::ADD_GROUP_MSG,
                             std::bind(&ChatService::addGroup, this, std::placeholders::_1, std::placeholders::_2,
                                       std::placeholders::_3) });
    m_msgHandlerMap.insert({ MsgType::GROUP_CHAT_MSG,
                             std::bind(&ChatService::groupChat, this, std::placeholders::_1, std::placeholders::_2,
                                       std::placeholders::_3) });
    m_msgHandlerMap.insert({ MsgType::LOGOUT_MSG,
                             std::bind(&ChatService::logout, this, std::placeholders::_1, std::placeholders::_2,
                                       std::placeholders::_3) });

    //连接Redis服务器
    if (m_redis.connect()) {
        //设置上报消息的回调
        m_redis.initNotifyHandler(
                                  std::bind(&ChatService::handleRedisSubscribeMessage, this, std::placeholders::_1,
                                            std::placeholders::_2));
    }
}
