#include "usermanager.h"
#include "networkcommunication.h"
#include "timemanager.h"
#include "chatclient.h"

#include <thread>
#include <iostream>

int main(int argc, char** argv) {
    if (argc < 3) {
        std::cerr << "Command invalid! Example: ./ChatClient [ip] [port]" << std::endl;
        exit(-1);
    }

    char* ip = argv[1];
    uint16_t port = std::atoi(argv[2]);

    int clientfd = NetworkCommunication::createClientSocket(ip, port);
    sem_init(&ChatClient::rwsem, 0, 0);

    std::thread readTask(ChatClient::readTaskHandler, clientfd);
    readTask.detach();

    while (true) {
        int choice;
        std::cout << "========================" << std::endl;
        std::cout << "1. Login" << std::endl;
        std::cout << "2. Register" << std::endl;
        std::cout << "3. Quit" << std::endl;
        std::cout << "========================" << std::endl;
        std::cout << "Choice: ";
        std::cin >> choice;
        std::cin.get(); // Read newline character

        switch (choice) {
            // login业务
            case 1: {
                int id{};
                char pwd[50]{};
                std::cout << "userid:";
                std::cin >> id;
                std::cin.get(); // 读掉缓冲区残留的回车

                std::cout << "userpassword:";
                std::cin.getline(pwd, 50);

                json js;
                js["msgtype"] = static_cast<int>(MsgType::LOGIN_MSG);
                js["id"] = id;
                js["password"] = pwd;

                std::string request = js.dump();

                ChatClient::g_isLoginSuccess = false;

                auto len = send(clientfd, request.c_str(), request.size(), 0);
                if (len == -1) {
                    std::cerr << "send login msg error:" << request << std::endl;
                }

                // 等待信号量，由子线程处理完登录的响应消息后，通知这里
                sem_wait(&ChatClient::rwsem);

                if (ChatClient::g_isLoginSuccess) {
                    // Enter the chat main menu
                    ChatClient::isMainMenuRunning = true;
                    ChatClient::mainMenu(clientfd);
                }

                break;
            }

                // register业务
            case 2: {
                char name[50]{};
                char pwd[50]{};

                std::cout << "username:";
                std::cin.getline(name, 50);
                std::cout << "userpassword:";
                std::cin.getline(pwd, 50);

                json js;
                js["msgtype"] = static_cast<int>(MsgType::REG_MSG);
                js["name"] = name;
                js["password"] = pwd;
                std::string request = js.dump();

                auto len = send(clientfd, request.c_str(), strlen(request.c_str()) + 1, 0);
                if (len == -1) {
                    std::cerr << "send reg msg error:" << request << std::endl;
                }

                // Wait for the semaphore, child thread will notify when handling the registration response
                sem_wait(&ChatClient::rwsem);

                break;
            }

                // quit业务
            case 3:
                close(clientfd);
                sem_destroy(&ChatClient::rwsem);
                exit(0);

            default:
                std::cerr << "Invalid input!" << std::endl;
                break;
        }
    }

    return 0;
}

