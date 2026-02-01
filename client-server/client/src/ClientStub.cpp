#include "../include/ClientStub.h"
#include "../../common/include/RobotOrder.h"

void ClientStub::init(std::string ip, int port) {
    socket.connect(ip, port);
}

void ClientStub::order(RobotOrder details) {
    socket.send(details);
}