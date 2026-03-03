#ifndef SERVERSTUB_H
#define SERVERSTUB_H

#include <condition_variable>
#include <future>
#include <mutex>
#include <queue>

#include "../../common/include/CustomerRecords.h"
#include "../../common/include/Robot.h"
#include "../../common/include/RobotOrder.h"
#include "./ServerState.h"
#include "./ServerSocket.h"

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
    ServerStub();

    void init(ServerSocket* socket, const ServerConfig& config);

    // Engineer thread: receives identification, then acts as engineer or IFA.
    void handle_client_request(int client_fd, int engineer_id,
                               AdminRequestQueue& adminQueue);

    // Admin thread (PFA): dequeues requests, replicates, commits, fulfills promise.
    void admin_process_requests(int admin_id, AdminRequestQueue& adminQueue);

    bool ReceiveRequest(RobotOrder& request, int client_fd);
    bool ShipRobot(const Robot& robot, int client_fd);
    bool ReturnRecord(const CustomerRecord& record, int client_fd);
    // PFA → IFA: send replication request to peer at peer_index.
    bool SendReplicationRequest(const ReplicationRequest& request,
                                int peer_index);
    // IFA: receive replication request from the PFA connection.
    bool ReceiveReplicationRequest(ReplicationRequest& request, int client_fd);
    // IFA → PFA: send one-int ack.
    bool SendReplicationResponse(int client_fd);
    // PFA: receive ack from peer at peer_index.
    bool ReceiveReplicationResponse(int peer_index);

   private:
    // Engineer role: handles a robot-order request (request_type == 1).
    bool handle_robot_order(const RobotOrder& request, int engineer_id,
                            int client_fd, AdminRequestQueue& adminQueue);

    // Engineer role: handles a record-read request (request_type == 2).
    bool handle_record_read(const RobotOrder& request, int engineer_id,
                            int client_fd);

    // IFA role: receives replication requests from PFA and responds.
    void handle_replication_request(int client_fd);

    ServerSocket* socket;
    ServerConfig config;

    CustomerRecords customerRecords;

    // Protects customerRecords.
    std::mutex records_mutex;

    // Protects serverState and smr_log.
    std::mutex state_mutex;

    ServerState serverState;
    StateMachineLog smr_log;

    // True once the PFA has connected to all peers.
    bool peers_connected;
};

#endif