#ifndef CHAT_NETWORKCOMMUNICATION_H
#define CHAT_NETWORKCOMMUNICATION_H

#include <string>

class NetworkCommunication {
public:
    static int createClientSocket(const char* ip, uint16_t port);
    static bool sendMessage(int clientfd, const std::string& message);
    static int receiveMessage(int clientfd, char* buffer, int bufferSize);
};

#endif //CHAT_NETWORKCOMMUNICATION_H
