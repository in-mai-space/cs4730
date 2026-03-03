#ifndef SERVERSTUB_H
#define SERVERSTUB_H

#include <condition_variable>
#include <future>
#include <mutex>
#include <queue>

#include "../../common/include/CustomerRecords.h"
#include "../../common/include/Robot.h"
#include "../../common/include/RobotOrder.h"
#include "../../common/include/StateMachineLog.h"
#include "ServerSocket.h"

// Request struct passed from engineer to admin
struct AdminRequest {
    Robot robot;
    std::promise<Robot> promise;
};

struct AdminRequestQueue {
    std::queue<AdminRequest> jobQueue;
    std::mutex mtx;
    std::condition_variable cv;
};

class ServerStub {
   public:
    void init(ServerSocket* socket);

    // Engineer thread: handles one client connection until disconnect.
    void handle_client_request(int client_fd, int engineer_id,
                               AdminRequestQueue& adminQueue);

    // Admin thread: dequeues requests, updates log+map, fulfills promise.
    void admin_process_requests(int admin_id, AdminRequestQueue& adminQueue);

    // ---- Client-facing transport stubs ----
    bool ReceiveRequest(RobotOrder& request, int client_fd);
    bool ShipRobot(const Robot& robot, int client_fd);
    bool ReturnRecord(const CustomerRecord& record, int client_fd);

   private:
    ServerSocket* socket;

    CustomerRecords customerRecords;
    std::mutex records_mutex;

    StateMachineLog smr_log;
};

#endif