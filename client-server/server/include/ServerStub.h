#ifndef SERVERSTUB_H
#define SERVERSTUB_H
#include "../../common/include/RobotOrder.h"
#include "../../common/include/Robot.h"

class ServerStub {
    public:
        void Init(int sock_fd);
        void ReceiveOrder(RobotOrder details);
        void ShipRobot(Robot robot);
};

#endif