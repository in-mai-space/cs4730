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
    /**
     * Initializes the client stub by connecting to the server at the specified IP and port.
     * @param ip The server IP address.
     * @param port The server port number.
     */
    void init(const std::string& ip, int port);

    /**
     * Sends a robot order request to the server and waits for the response.
     * @param request The RobotOrder request to send.
     * @param customer_id The ID of the customer making the request.
     * @param recorder The LatencyRecorder to record the latency of the operation.
     */
    bool Order(const RobotOrder& request, int customer_id,
               LatencyRecorder& recorder);

    /**
     * Sends a read-record request to the server and waits for the response.
     * @param request The RobotOrder request to send (with request_type set to 2).
     */
    bool ReadRecords(int customer_id, int orders, LatencyRecorder& recorder);

    /**
     * Sends a scan-records request to the server and waits for the response.
     * @param max_customer_id The maximum customer ID to scan up to.
     * @param recorder The LatencyRecorder to record the latency of the operation.
     */
    bool ScanRecords(int max_customer_id, LatencyRecorder& recorder);

    /**
     * Sends a read-record request to the server and returns the CustomerRecord response.
     * @param request The RobotOrder request to send (with request_type set to 2).
     */
    CustomerRecord ReadRecord(const RobotOrder& request);

   private:
    CustomerRecord read_record(const RobotOrder& request);
    Robot order_robot(const RobotOrder& request);
    CustomerRecord scan_records(const RobotOrder& request);

    ClientSocket socket;
    std::atomic<bool> running{true};
};

#endif