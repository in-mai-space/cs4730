#include "../include/ServerStub.h"
#include "../../common/include/RobotOrder.h"
#include "../../common/include/Robot.h"

void ServerStub::Init(int sock_fd) {
    // Implementation for initializing the server stub with the given socket file descriptor
}

void ServerStub::ReceiveOrder(RobotOrder details) {
    // Implementation for receiving an order from the client
}

void ServerStub::ShipRobot(Robot robot) {
    // Implementation for shipping a robot to the client
}
