#include "networkcommunication.h"

#include <iostream>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "cstring"

int NetworkCommunication::createClientSocket(const char* ip, uint16_t port) {
    int clientfd = socket(AF_INET, SOCK_STREAM, 0);
    if (-1 == clientfd) {
        std::cerr << "socket create error" << std::endl;
        exit(-1);
    }

    sockaddr_in server;
    memset(&server, 0, sizeof(sockaddr_in));
    server.sin_family = AF_INET;
    server.sin_port = htons(port);
    server.sin_addr.s_addr = inet_addr(ip);

    if (-1 == connect(clientfd, (sockaddr*) &server, sizeof(sockaddr_in))) {
        std::cerr << "connect server error" << std::endl;
        close(clientfd);
        exit(-1);
    }

    return clientfd;
}

bool NetworkCommunication::sendMessage(int clientfd, const std::string& message) {
    int len = send(clientfd, message.c_str(), strlen(message.c_str()) + 1, 0);
    if (len == -1) {
        std::cerr << "send message error:" << message << std::endl;
        return false;
    }

    return true;
}

int NetworkCommunication::receiveMessage(int clientfd, char* buffer, int bufferSize) {
    return recv(clientfd, buffer, bufferSize, 0);
}