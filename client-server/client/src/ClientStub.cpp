#include "../include/ClientStub.h"
#include "../../common/include/RobotOrder.h"

void ClientStub::Init(std::string ip, int port) {
    socket.Connect(ip, port);
}

void ClientStub::Order(RobotOrder details) {
    socket.Send(details);
}