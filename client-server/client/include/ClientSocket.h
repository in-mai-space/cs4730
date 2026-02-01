#ifndef CLIENTSOCKET_H
#define CLIENTSOCKET_H

#include "../../common/include/RobotOrder.h"
#include <string>
#include <cstddef>

class ClientSocket {
public:
    ClientSocket();
    ~ClientSocket();

    bool connect(const std::string& ip, int port);
    bool send(const RobotOrder& order);
    bool receive(RobotOrder& order);

private:
    int marshall(const RobotOrder& order, char* buffer, int buffer_size);
    int unmarshall(const char* buffer, int buffer_size, RobotOrder& order);

    bool send_all(const char* data, size_t len);
    bool receive_all(char* data, size_t len);

    int sock_fd;
};

#endif
