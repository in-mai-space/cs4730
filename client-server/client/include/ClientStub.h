#ifndef CLIENTSTUB_H
#define CLIENTSTUB_H
#include <string>
#include <vector>
#include <chrono>
#include <mutex>
#include "../../common/include/RobotOrder.h"
#include "../../common/include/Robot.h"
#include "ClientSocket.h"

class ClientStub {
    public:
        void init(std::string ip, int port);
        void order(RobotOrder details, int customer_id, std::vector<long long>& latencies, std::mutex& latency_mutex);

    private:
        ClientSocket socket;
};

#endif