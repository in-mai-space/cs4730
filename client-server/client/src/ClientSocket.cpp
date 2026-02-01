#include "../include/ClientSocket.h"
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>

ClientSocket::ClientSocket() : sockfd(-1) {}

ClientSocket::~ClientSocket() {
    if (sockfd >= 0)
        close(sockfd);
}

bool ClientSocket::Connect(const std::string& ip, int port) {
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
        return false;

    sockaddr_in serv_addr{};
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);

    if (inet_pton(AF_INET, ip.c_str(), &serv_addr.sin_addr) <= 0) {
        close(sockfd);
        sockfd = -1;
        return false;
    }

    if (connect(sockfd, (sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        close(sockfd);
        sockfd = -1;
        return false;
    }

    return true;
}

bool ClientSocket::Send(const RobotOrder& order) {
    char buffer[sizeof(RobotOrder)];
    int len = Marshall(order, buffer, sizeof(buffer));
    if (len <= 0)
        return false;

    return sendAll(buffer, len);
}

bool ClientSocket::Receive(RobotOrder& order) {
    char buffer[sizeof(RobotOrder)];

    if (!recvAll(buffer, sizeof(buffer)))
        return false;

    return Unmarshal(buffer, sizeof(buffer), order) > 0;
}

bool ClientSocket::sendAll(const char* data, size_t len) {
    size_t total = 0;
    while (total < len) {
        ssize_t sent = send(sockfd, data + total, len - total, 0);
        if (sent <= 0)
            return false;
        total += sent;
    }
    return true;
}

bool ClientSocket::recvAll(char* data, size_t len) {
    size_t total = 0;
    while (total < len) {
        ssize_t recvd = recv(sockfd, data + total, len - total, 0);
        if (recvd <= 0)
            return false;
        total += recvd;
    }
    return true;
}

int ClientSocket::Marshall(const RobotOrder& order, char* buffer, int buffer_size) {
    if (buffer_size < (int)sizeof(RobotOrder))
        return -1;

    std::memcpy(buffer, &order, sizeof(RobotOrder));
    return sizeof(RobotOrder);
}

int ClientSocket::Unmarshal(const char* buffer, int buffer_size, RobotOrder& order) {
    if (buffer_size < (int)sizeof(RobotOrder))
        return -1;

    std::memcpy(&order, buffer, sizeof(RobotOrder));
    return sizeof(RobotOrder);
}
