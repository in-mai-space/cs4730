#ifndef CLIENTSTUB_H
#define CLIENTSTUB_H
#include <string>
#include "../../common/include/RobotOrder.h"
#include "ClientSocket.h"

class ClientStub {
    public:
        void Init(std::string ip, int port);
        void Order(RobotOrder details);

    private:
        ClientSocket socket;
};

#endif