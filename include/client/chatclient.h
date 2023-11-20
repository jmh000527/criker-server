#ifndef CHAT_CHATCLIENT_H
#define CHAT_CHATCLIENT_H

#include <semaphore.h>
#include <semaphore>
#include <atomic>
#include "chatservice.h"
#include "usermanager.h"

class ChatClient {
public:
    static bool isMainMenuRunning;
    static std::atomic_bool g_isLoginSuccess;
    static std::counting_semaphore<1> rwsem;

    static void readTaskHandler(int clientfd);
    static void mainMenu(int clientfd);
    static void showCurrentUserData();

    static void doRegResponse(json& responsejs);
    static void doLoginResponse(json& responsejs);

    static void help(int fd = 0, const std::string& str = "");
    static void chat(int, const std::string&);
    static void addfriend(int, const std::string&);
    static void creategroup(int, const std::string&);
    static void addgroup(int, const std::string&);
    static void groupchat(int, const std::string&);
    static void logout(int clientfd, const std::string&);
};


#endif //CHAT_CHATCLIENT_H
