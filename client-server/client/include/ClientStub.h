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
        /**
         * Initializes the client stub by connecting to the server.
         * @param ip The server IP address.
         * @param port The server port number.
         */
        void init(std::string ip, int port);

        /**
         * Places orders to the server and records latencies.
         * @param details The template RobotOrder containing order details.
         * @param customer_id The ID of the customer placing the orders.
         * @param latencies A vector to store the latencies of each order.
         * @param latency_mutex A mutex to protect access to the latencies vector.
         */
        void order(RobotOrder details, int customer_id, std::vector<long long>& latencies, std::mutex& latency_mutex);

    private:
        ClientSocket socket;
};

#endif