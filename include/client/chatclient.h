#ifndef CHAT_CHATCLIENT_H
#define CHAT_CHATCLIENT_H

#include <semaphore.h>
#include <atomic>
#include "chatservice.h"
#include "usermanager.h"

class ChatClient {
public:
    static bool isMainMenuRunning;
    static std::atomic_bool g_isLoginSuccess;
    static sem_t rwsem;

    static void readTaskHandler(int clientfd);
    static void mainMenu(int clientfd);
    static void showCurrentUserData();

    static void doRegResponse(json& responsejs);
    static void doLoginResponse(json& responsejs);
    
    static void help(int fd = 0, std::string str = "");
    static void chat(int, std::string);
    static void addfriend(int, std::string);
    static void creategroup(int, std::string);
    static void addgroup(int, std::string);
    static void groupchat(int, std::string);
    static void logout(int clientfd, std::string);

};


#endif //CHAT_CHATCLIENT_H
