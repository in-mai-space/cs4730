#ifndef CLIENTSTUB_H
#define CLIENTSTUB_H
#include <chrono>
#include <mutex>
#include <string>
#include <vector>

#include "../../common/include/CustomerRecords.h"
#include "../../common/include/Robot.h"
#include "../../common/include/RobotOrder.h"
#include "ClientSocket.h"

struct LatencyRecorder {
    std::vector<long long> latencies;
    std::mutex mutex;
};

class ClientStub {
   public:
    void init(const std::string& ip, int port);

    /**
     * Order: sends `orders` robot-order requests (request_type=1) and
     * receives robot information for each. Records latencies.
     */
    bool Order(const RobotOrder& tmpl, int customer_id,
               LatencyRecorder& recorder);

    /**
     * ReadRecord: sends a single customer-record read request (request_type=2)
     * and receives the CustomerRecord. Returns the record.
     */
    CustomerRecord ReadRecord(const RobotOrder& request);

   private:
    ClientSocket socket;
};

#endif