#ifndef SERVERSOCKET_H
#define SERVERSOCKET_H
#include <cstddef>

#include "../../common/include/Robot.h"
#include "../../common/include/RobotOrder.h"

class ServerSocket {
   public:
    ServerSocket();
    ~ServerSocket();

    bool listen(int port);
    int accept();
    bool send(const Robot& robot, int client_fd = -1);
    bool receive(RobotOrder& order, int client_fd = -1);

    void set_client_fd(int client_fd) { this->client_fd = client_fd; }

   private:
    int marshall(const Robot& robot, char* buffer, int buffer_size);
    int unmarshall(const char* buffer, int buffer_size, RobotOrder& order);

    bool send_all(const char* data, size_t len, int client_fd);
    bool receive_all(char* data, size_t len, int client_fd);

    int socket_fd;
    int client_fd;
};

#endif