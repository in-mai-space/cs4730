#ifndef SERVERSTUB_H
#define SERVERSTUB_H

#include <condition_variable>
#include <future>
#include <mutex>
#include <queue>

#include "../../common/include/CustomerRecords.h"
#include "../../common/include/Robot.h"
#include "../../common/include/RobotOrder.h"
#include "./ServerSocket.h"
#include "./ServerState.h"

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

    /**
     * Handles a client request. This method is called by engineer threads to process incoming requests from clients. 
     */
    void HandleClientRequest(int client_fd, int engineer_id,
                               AdminRequestQueue& adminQueue);

    /**
     * Processes admin requests. This method is run by the single admin thread to process incoming requests from engineers, replicate them to peers, and fulfill promises back to engineers.
     */
    void AdminProcessRequests(int admin_id, AdminRequestQueue& adminQueue);

    /**
     * Processes a RobotOrder request from a client. If it's a new order, it will be processed through the PFA protocol. If it's a record read, it will return the requested record.
     */
    bool ReceiveRequest(RobotOrder& request, int client_fd);

    /**
     * Sends a Robot response back to the client.
     */
    bool ShipRobot(const Robot& robot, int client_fd);

    /**
     * Sends a CustomerRecord response back to the client.
     */
    bool ReturnRecord(const CustomerRecord& record, int client_fd);

    /**
     * Sends a replication request to the specified peer and waits for an acknowledgement. Returns true if the peer acknowledged successfully, false otherwise.
     */
    bool SendReplicationRequest(const ReplicationRequest& request,
                                int peer_index);

    /**
     * Sends a replication response (acknowledgement) back to the peer that sent the replication request.
     */
    bool ReceiveReplicationRequest(ReplicationRequest& request, int client_fd);

    /**
     * Receives a replication response (acknowledgement) from the specified peer. Returns true if the acknowledgement was received successfully, false otherwise.
     */
    bool SendReplicationResponse(int client_fd);

    /**
     * Receives a replication response (acknowledgement) from the specified peer. Returns true if the acknowledgement was received successfully, false otherwise.
     */
    bool ReceiveReplicationResponse(int peer_index);

   private:
    bool handle_robot_order(const RobotOrder& request, int engineer_id,
                            int client_fd, AdminRequestQueue& adminQueue);

    bool handle_record_read(const RobotOrder& request, int engineer_id,
                            int client_fd);

    AdminRequest wait_for_admin_request(AdminRequestQueue& adminQueue);

    void ensure_primary_and_connect_peers();

    int append_to_log(const Robot& robot);

    void replicate_to_peers(const Robot& robot, int cur_last);

    void commit_locally(const Robot& robot, int cur_last);

    void fulfill_promise(AdminRequest& req, int admin_id);

    void handle_replication_request(int client_fd);

    void handle_pfa_disconnect(int client_fd);

    void log_replication_request(const ReplicationRequest& req);

    void apply_replication_entry(const ReplicationRequest& req);

    void apply_committed_entry(const ReplicationRequest& req);

    bool send_replication_ack(int client_fd);

    bool process_request(const RobotOrder& request, int client_fd,
                         int engineer_id, AdminRequestQueue& adminQueue);

    ServerSocket* socket;
    ServerConfig config;

    CustomerRecords customerRecords;

    // Protects customerRecords
    std::mutex records_mutex;

    // Protects server_state and smr_log
    std::mutex state_mutex;

    ServerState server_state;
    StateMachineLog smr_log;

    bool peers_connected;
};

#endif