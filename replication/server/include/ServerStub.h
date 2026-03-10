#ifndef SERVERSTUB_H
#define SERVERSTUB_H

#include <condition_variable>
#include <future>
#include <mutex>
#include <queue>

#include "../../common/include/CustomerRecords.h"
#include "../../common/include/Robot.h"
#include "../../common/include/RobotOrder.h"
#include "./ServerConfig.h"
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
     * Handles a client request. This method is called by engineer threads to
     * process incoming requests from clients.
     * @param client_fd The file descriptor for the client connection.
     * @param engineer_id The ID of the engineer thread handling this request.
     * @param adminQueue The queue to submit admin requests to.
     */
    void HandleClientRequest(int client_fd, int engineer_id,
                             AdminRequestQueue& adminQueue);

    /**
     * Processes admin requests. This method is run by the single admin thread
     * to process incoming requests from engineers, replicate them to peers, and
     * fulfill promises back to engineers.
     * @param admin_id The ID of the admin thread (for logging purposes).
     * @param adminQueue The queue to receive admin requests from engineer
     * threads.
     */
    void AdminProcessRequests(int admin_id, AdminRequestQueue& adminQueue);

    /**
     * Processes a RobotOrder request from a client. If it's a new order, it
     * will be processed through the PFA protocol. If it's a record read, it
     * will return the requested record.
     * @param request The RobotOrder request received from the client.
     * @param client_fd The file descriptor for the client connection.
     */
    bool ReceiveRequest(RobotOrder& request, int client_fd);

    /**
     * Sends a Robot response back to the client.
     * @param robot The Robot object to send back to the client.
     * @param client_fd The file descriptor for the client connection.
     */
    bool ShipRobot(const Robot& robot, int client_fd);

    /**
     * Sends a CustomerRecord response back to the client.
     * @param record The CustomerRecord object to send back to the client.
     * @param client_fd The file descriptor for the client connection.
     */
    bool ReturnRecord(const CustomerRecord& record, int client_fd);

    /**
     * Sends a replication request to the specified peer and waits for an
     * acknowledgement. Returns true if the peer acknowledged successfully,
     * false otherwise.
     * @param request The ReplicationRequest to send to the peer.
     * @param peer_index The index of the peer to send the request to.
     */
    bool SendReplicationRequest(const ReplicationRequest& request,
                                int peer_index);

    /**
     * Receives a replication request from a peer. Returns true if the request
     * was received successfully, false otherwise.
     * @param request The ReplicationRequest object to populate with the
     * received request data.
     * @param client_fd The file descriptor for the peer connection to receive
     * the request from.
     */
    bool ReceiveReplicationRequest(ReplicationRequest& request, int client_fd);

    /**
     * Sends a replication response (acknowledgement) back to the peer that sent
     * the replication request.
     * @param client_fd The file descriptor for the peer connection to send the
     * acknowledgement to.
     */
    bool SendReplicationResponse(int client_fd);

    /**
     * Receives a replication response (acknowledgement) from a peer. Returns
     * true if the acknowledgement was received successfully, false otherwise.
     * @param peer_index The index of the peer to receive the acknowledgement
     * from.
     */
    bool ReceiveReplicationResponse(int peer_index);

   private:
    bool handle_robot_order(const RobotOrder& request, int engineer_id,
                            int client_fd, AdminRequestQueue& adminQueue);

    bool handle_record_read(const RobotOrder& request, int client_fd);

    AdminRequest wait_for_admin_request(AdminRequestQueue& adminQueue);

    void ensure_primary_and_connect_peers();

    int append_to_log(const Robot& robot);

    void replicate_to_peers(int cur_last);

    void commit_locally(const Robot& robot, int cur_last);

    void fulfill_promise(AdminRequest& req, int admin_id);

    void handle_replication_request(int client_fd);

    void handle_pfa_disconnect();

    void handle_ifa_disconnect(int peer_index);

    void log_replication_request(const ReplicationRequest& req);

    void apply_replication_entry(const ReplicationRequest& req);

    void apply_committed_entry(const ReplicationRequest& req);

    bool send_replication_ack(int client_fd);

    bool process_request(const RobotOrder& request, int client_fd,
                         int engineer_id, AdminRequestQueue& adminQueue);

    bool try_reconnect_and_catchup(int peer_index);

    ServerSocket* socket;
    ServerConfig config;

    CustomerRecords customerRecords;

    std::mutex records_mutex;  // protects customerRecords

    std::mutex state_mutex;  // protects server_state and smr_log

    ServerState server_state;
    StateMachineLog smr_log;

    bool peers_connected;
    std::atomic<bool> running{true};

    std::vector<bool> peer_alive;
    std::vector<int> peer_last_index;
};

#endif