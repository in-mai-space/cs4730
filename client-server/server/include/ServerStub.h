#ifndef SERVERSTUB_H
#define SERVERSTUB_H
#include "../../common/include/RobotOrder.h"
#include "../../common/include/Robot.h"
#include "ServerSocket.h"

class ServerStub {
    public:
        void init(ServerSocket* socket);
        void handle_client(int client_fd);
        Robot process_order(const RobotOrder& order);

    private:
        ServerSocket* socket;
};

#endif