#ifndef CLIENTSTUB_H
#define CLIENTSTUB_H
#include <chrono>
#include <mutex>
#include <string>
#include <vector>

#include "../../common/include/Robot.h"
#include "../../common/include/RobotOrder.h"
#include "ClientSocket.h"

struct LatencyRecorder {
    std::vector<long long> latencies;
    std::mutex mutex;
};

class ClientStub {
   public:
    /**
     * Initializes the client stub by connecting to the server.
     * @param ip The server IP address.
     * @param port The server port number.
     */
    void init(const std::string& ip, int port);

    /**
     * Places orders to the server and records latencies.
     * @param details The template RobotOrder containing order details.
     * @param customer_id The ID of the customer placing the orders.
     * @param recorder A LatencyRecorder struct to store the latencies of each
     * order.
     * @param latency_mutex A mutex to protect access to the latencies vector.
     */
    bool order(const RobotOrder& details, int customer_id,
               LatencyRecorder& recorder);

   private:
    ClientSocket socket;
};

#endif