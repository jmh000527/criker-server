#include "chatclient.h"

#include "usermanager.h"
#include "networkcommunication.h"
#include "timemanager.h"
#include "json.hpp"
#include "msgtype.h"
#include "utils.h"

#include <iostream>
#include <thread>
#include <string>
#include <vector>

using json = nlohmann::json;

bool ChatClient::isMainMenuRunning;
std::atomic_bool ChatClient::g_isLoginSuccess;
sem_t ChatClient::rwsem;

// 系统支持的客户端命令列表
std::unordered_map<std::string, std::string> commandMap = {
    { "help",        "显示所有支持的命令，格式：help" },
    { "chat",        "一对一聊天，格式：chat:friendid:message" },
    { "addfriend",   "添加好友，格式：addfriend:friendid" },
    { "creategroup", "创建群组，格式：creategroup:groupname:groupdesc" },
    { "addgroup",    "加入群组，格式：addgroup:groupid" },
    { "groupchat",   "群聊，格式：groupchat:groupid:message" },
    { "logout",      "注销，格式：logout" }};

// 注册系统支持的客户端命令处理
std::unordered_map<std::string, std::function<void(int, std::string)>> commandHandlerMap{
    { "help",        &ChatClient::help },
    { "chat",        &ChatClient::chat },
    { "addfriend",   &ChatClient::addfriend },
    { "creategroup", &ChatClient::creategroup },
    { "addgroup",    &ChatClient::addgroup },
    { "groupchat",   &ChatClient::groupchat },
    { "logout",      &ChatClient::logout }};

void ChatClient::readTaskHandler(int clientfd) {
    for (;;) {
        char buffer[1024] = { 0 };
        auto len = NetworkCommunication::receiveMessage(clientfd, buffer, 1024);
        if (-1 == len || 0 == len) {
            close(clientfd);
            exit(-1);
        }

        // 接受ChatServer转发的数据并反序列化
        json js = json::parse(buffer);
        MsgType msgtype{ static_cast<MsgType>(getValueFromJson<int>(js, "msgtype")) };

        if (MsgType::ONE_CHAT_MSG == msgtype) {
            std::cout << getValueFromJson<std::string>(js, "time") << " [" << getValueFromJson<int>(js, "id") << "]"
                      << getValueFromJson<std::string>(js, "name") << " said: "
                      << getValueFromJson<std::string>(js, "msg")
                      << std::endl;
            continue;
        }

        if (MsgType::GROUP_CHAT_MSG == msgtype) {
            std::cout << "Group Message[" << getValueFromJson<int>(js, "groupid") << "]:"
                      << getValueFromJson<std::string>(js, "time") << " [" << getValueFromJson<int>(js, "id") << "]"
                      << getValueFromJson<std::string>(js, "name") << " said: "
                      << getValueFromJson<std::string>(js, "msg")
                      << std::endl;
            continue;
        }

        if (MsgType::LOGIN_MSG_ACK == msgtype) {
            doLoginResponse(js);
            sem_post(&rwsem);
            continue;
        }

        if (MsgType::REG_MSG_ACK == msgtype) {
            doRegResponse(js);
            sem_post(&rwsem);
            continue;
        }
    }
}

void ChatClient::mainMenu(int clientfd) {
    help();

    char buffer[1024] = { 0 };
    while (isMainMenuRunning) {
        std::cin.getline(buffer, 1024);
        std::string commandbuf(buffer);
        std::string command;

        int index = commandbuf.find(":");
        if (std::string::npos == index) {
            command = commandbuf;
        } else {
            command = commandbuf.substr(0, index);
        }

        auto iter = commandHandlerMap.find(command);
        if (iter == std::end(commandHandlerMap)) {
            std::cerr << "invalid input command!" << std::endl;
            continue;
        }

        iter->second(clientfd, commandbuf.substr(index + 1, commandbuf.size() - index));
    }
}

void ChatClient::showCurrentUserData() {
    // Display the user data using UserManager
    User currentUser = UserManager::getCurrentUser();
    std::cout << "======================login user======================" << std::endl;
    std::cout << "current login user => id:" << currentUser.getId() << " name:" << currentUser.getName()
              << std::endl;

    std::cout << "----------------------friend list---------------------" << std::endl;
    std::vector<User> friendList = UserManager::getCurrentUserFriendList();
    if (!friendList.empty()) {
        for (const User& user: friendList) {
            std::cout << user.getId() << " " << user.getName() << " " << user.getState() << std::endl;
        }
    }

    std::cout << "----------------------group list----------------------" << std::endl;
    std::vector<Group> groupList = UserManager::getCurrentUserGroupList();
    if (!groupList.empty()) {
        for (const Group& group: groupList) {
            std::cout << group.getId() << " " << group.getName() << " " << group.getDesc() << std::endl;
            const std::vector<GroupUser>& users = group.getUsers();
            for (const GroupUser& user: users) {
                std::cout << user.getId() << " " << user.getName() << " " << user.getState()
                          << " " << user.getRole() << std::endl;
            }
        }
    }
    std::cout << "======================================================" << std::endl;
}

void ChatClient::doRegResponse(json& responsejs) {
    // Process registration response
    if (0 != responsejs["errno"].get<int>()) {
        std::cerr << "name is already exist, register error!" << std::endl;
    } else {
        std::cout << "name register success, userid is " << responsejs["id"]
                  << ", do not forget it!" << std::endl;
    }
}

void ChatClient::doLoginResponse(json& responsejs) {
    if (0 != responsejs["errno"].get<int>()) {
        // 登录失败
        std::cerr << responsejs["errmsg"] << std::endl;
        g_isLoginSuccess = false;
    } else {
        // 登录成功

        User currentUser;
        currentUser.setId(responsejs["id"].get<int>());
        currentUser.setName(responsejs["name"]);
        UserManager::setCurrentUser(currentUser);

        // 记录当前用户的好友列表信息
        if (responsejs.contains("friends")) {
            std::vector<User> friends;

            for (const std::string& str: responsejs["friends"]) {
                json js = json::parse(str);

                User user;
                user.setId(js["id"].get<int>());
                user.setName(js["name"]);
                user.setState(js["state"]);
                friends.push_back(std::move(user));
            }

            UserManager::setCurrentUserFriendList(friends);
        }

        // 记录当前用户的群组列表信息
        if (responsejs.contains("groups")) {
            std::vector<Group> groups;

            for (const std::string& groupstr: responsejs["groups"]) {
                json grpjs = json::parse(groupstr);
                Group group;
                group.setId(grpjs["id"].get<int>());
                group.setName(grpjs["groupname"]);
                group.setDesc(grpjs["groupdesc"]);

                std::vector<GroupUser> users;
                for (const std::string& userstr: grpjs["users"]) {
                    json js = json::parse(userstr);
                    GroupUser user{ js["id"].get<int>(), js["name"], "", js["state"], js["role"] };
                    users.push_back(user);
                }

                group.setUsers(users);
                groups.push_back(group);
            }

            UserManager::setCurrentUserGroupList(groups);
        }

        // 显示登录用户的基本信息
        showCurrentUserData();

        // 显示当前用户的离线消息  个人聊天信息或者群组消息
        if (responsejs.contains("offlinemsg")) {
            const std::vector<std::string>& offlineMessages = responsejs["offlinemsg"];
            for (const std::string& str: offlineMessages) {
                json js = json::parse(str);
                if (MsgType::ONE_CHAT_MSG == static_cast<MsgType>(getValueFromJson<int>(js, "msgtype"))) {
                    std::cout << getValueFromJson<std::string>(js, "time") << " [" << getValueFromJson<int>(js, "id")
                              << "]" << getValueFromJson<std::string>(js, "name") << " said: "
                              << getValueFromJson<std::string>(js, "msg") << std::endl;
                } else {
                    std::cout << "群消息[" << getValueFromJson<int>(js, "groupid") << "]:"
                              << getValueFromJson<std::string>(js, "time") << " [" << getValueFromJson<int>(js, "id")
                              << "]" << getValueFromJson<std::string>(js, "name") << " said: "
                              << getValueFromJson<std::string>(js, "msg") << std::endl;
                }
            }
        }

        g_isLoginSuccess = true;
    }
}

// "help" command handler
void ChatClient::help(int, std::string) {
    std::cout << "show command list >>> " << std::endl;
    for (auto& p: commandMap) {
        std::cout << p.first << " : " << p.second << std::endl;
    }
    std::cout << std::endl;
}

// "addfriend" command handler
void ChatClient::addfriend(int clientfd, std::string str) {
    int friendid = std::atoi(str.c_str());

    json js;
    js["msgtype"] = static_cast<int>(MsgType::ADD_FRIEND_MSG);
    js["id"] = UserManager::getCurrentUser().getId();
    js["friendid"] = friendid;

    std::string buffer = js.dump();

    NetworkCommunication::sendMessage(clientfd, buffer);
}

// "chat" command handler
void ChatClient::chat(int clientfd, std::string str) {
    int index = str.find(":"); // friendid:message
    if (std::string::npos == index) {
        std::cerr << "chat command invalid!" << std::endl;
        return;
    }

    int friendid = std::atoi(str.substr(0, index).c_str());
    std::string message = str.substr(index + 1, str.size() - index);

    json js;
    js["msgtype"] = static_cast<int>(MsgType::ONE_CHAT_MSG);
    js["id"] = UserManager::getCurrentUser().getId();
    js["name"] = UserManager::getCurrentUser().getName();
    js["toid"] = friendid;
    js["msg"] = message;
    js["time"] = TimeManager::getCurrentTime();

    std::string buffer = js.dump();

    NetworkCommunication::sendMessage(clientfd, buffer);
}

// "creategroup" command handler  groupname:groupdesc
void ChatClient::creategroup(int clientfd, std::string str) {
    int index = str.find(":");
    if (-1 == index) {
        std::cerr << "creategroup command invalid!" << std::endl;
        return;
    }

    std::string groupname = str.substr(0, index);
    std::string groupdesc = str.substr(index + 1, str.size() - index);

    json js;
    js["msgtype"] = static_cast<int>(MsgType::CREATE_GROUP_MSG);
    js["id"] = UserManager::getCurrentUser().getId();
    js["groupname"] = groupname;
    js["groupdesc"] = groupdesc;
    std::string buffer = js.dump();

    NetworkCommunication::sendMessage(clientfd, buffer);
}

// "addgroup" command handler
void ChatClient::addgroup(int clientfd, std::string str) {
    int groupid = std::atoi(str.c_str());

    json js;
    js["msgtype"] = static_cast<int>(MsgType::ADD_GROUP_MSG);
    js["id"] = UserManager::getCurrentUser().getId();
    js["groupid"] = groupid;
    std::string buffer = js.dump();

    NetworkCommunication::sendMessage(clientfd, buffer);
}

// "groupchat" command handler   groupid:message
void ChatClient::groupchat(int clientfd, std::string str) {
    int index = str.find(":");
    if (-1 == index) {
        std::cerr << "groupchat command invalid!" << std::endl;
        return;
    }

    int groupid = std::atoi(str.substr(0, index).c_str());
    std::string message = str.substr(index + 1, str.size() - index);

    json js;
    js["msgtype"] = static_cast<int>(MsgType::GROUP_CHAT_MSG);
    js["id"] = UserManager::getCurrentUser().getId();
    js["name"] = UserManager::getCurrentUser().getName();
    js["groupid"] = groupid;
    js["msg"] = message;
    js["time"] = TimeManager::getCurrentTime();
    std::string buffer = js.dump();

    NetworkCommunication::sendMessage(clientfd, buffer);
}

// "logout" command handler
void ChatClient::logout(int clientfd, std::string) {
    json js;
    js["msgtype"] = static_cast<int>(MsgType::LOGOUT_MSG);
    js["id"] = UserManager::getCurrentUser().getId();
    js["name"] = UserManager::getCurrentUser().getName();
    std::string buffer = js.dump();

    if (NetworkCommunication::sendMessage(clientfd, buffer)) {
        ChatClient::isMainMenuRunning = false;
    }
}
