#ifndef SERVERSTUB_H
#define SERVERSTUB_H
#include "../../common/include/RobotOrder.h"
#include "../../common/include/Robot.h"

class ServerStub {
    public:
        void init(int sock_fd);
        void receive_order(RobotOrder details);
        void ship_robot(Robot robot);
};

#endif