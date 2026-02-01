#ifndef CLIENTSOCKET_H
#define CLIENTSOCKET_H

#include "../../common/include/RobotOrder.h"
#include <string>
#include <cstddef>

class ClientSocket {
public:
    ClientSocket();
    ~ClientSocket();

    bool Connect(const std::string& ip, int port);
    bool Send(const RobotOrder& order);
    bool Receive(RobotOrder& order);

private:
    int Marshall(const RobotOrder& order, char* buffer, int buffer_size);
    int Unmarshal(const char* buffer, int buffer_size, RobotOrder& order);

    bool sendAll(const char* data, size_t len);
    bool recvAll(char* data, size_t len);

    int sockfd;
};

#endif
