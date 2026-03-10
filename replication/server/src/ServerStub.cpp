#include "../include/ServerStub.h"

#include <unistd.h>

#include <chrono>
#include <future>
#include <iostream>
#include <mutex>
#include <queue>
#include <thread>

static const int MSG_IDENTIFY_PFA = 2;

ServerStub::ServerStub()
    : socket(nullptr), peers_connected(false), running(true) {}

void ServerStub::init(ServerSocket* socket, const ServerConfig& config) {
    this->socket = socket;
    this->config = config;
    this->customerRecords = CustomerRecords();
    this->peers_connected = false;

    this->server_state.factory_id = config.factory_id;
    this->server_state.primary_id = -1;
    this->server_state.last_index = 0;
    this->server_state.committed_index = 0;

    this->peer_last_index.assign(config.peers.size(), 0);
    this->peer_alive.assign(config.peers.size(), false);
}

void ServerStub::AdminProcessRequests(int admin_id,
                                      AdminRequestQueue& adminQueue) {
    std::cout << "[PFA " << admin_id
              << "] Admin thread started (factory_id=" << config.factory_id
              << ")" << std::endl;

    while (running) {
        AdminRequest req = wait_for_admin_request(adminQueue);

        ensure_primary_and_connect_peers();

        int cur_last = append_to_log(req.robot);
        replicate_to_peers(cur_last);
        commit_locally(req.robot, cur_last);

        {
            std::lock_guard<std::mutex> sl(state_mutex);
            std::cout << "[PFA " << admin_id
                      << "] last_index=" << server_state.last_index
                      << " committed_index=" << server_state.committed_index
                      << std::endl;
        }

        fulfill_promise(req, admin_id);
    }
}

AdminRequest ServerStub::wait_for_admin_request(AdminRequestQueue& adminQueue) {
    std::unique_lock<std::mutex> lock(adminQueue.mtx);
    adminQueue.cv.wait(lock, [&] { return !adminQueue.jobQueue.empty(); });

    AdminRequest req = std::move(adminQueue.jobQueue.front());
    adminQueue.jobQueue.pop();
    return req;
}

void ServerStub::ensure_primary_and_connect_peers() {
    std::lock_guard<std::mutex> sl(state_mutex);
    if (server_state.primary_id != -1) return;

    server_state.primary_id = server_state.factory_id;

    if (!peers_connected && !config.peers.empty()) {
        std::cout << "[PFA] Connecting to peers..." << std::endl;

        socket->ConnectToPeers(config.peers);
        peers_connected = true;

        for (size_t i = 0; i < peer_alive.size(); i++)
            peer_alive[i] = socket->IsPeerConnected(i);

        std::cout << "[PFA] Connected to peers." << std::endl;
    }
}

int ServerStub::append_to_log(const Robot& robot) {
    std::lock_guard<std::mutex> sl(state_mutex);
    smr_log.AddOperation(1, robot.customer_id, robot.order_number);
    server_state.last_index++;
    return server_state.last_index;
}

void ServerStub::replicate_to_peers(int cur_last) {
    int factory_id;
    int committed_index;

    {
        std::lock_guard<std::mutex> sl(state_mutex);
        factory_id = server_state.factory_id;
        committed_index = server_state.committed_index;
    }

    for (int i = 0; i < socket->NumPeers(); i++) {
        if (!peer_alive[i]) {
            if (!try_reconnect_and_catchup(i)) continue;
        }

        int start = peer_last_index[i] + 1;
        for (int idx = start; idx <= cur_last; idx++) {
            MapOp op = smr_log.GetOperation(idx);
            ReplicationRequest rep;
            rep.factory_id = factory_id;
            rep.committed_index = committed_index;
            rep.last_index = idx;
            rep.operation = op;

            if (!SendReplicationRequest(rep, i) ||
                !ReceiveReplicationResponse(i)) {
                handle_ifa_disconnect(i);
                break;
            }

            peer_last_index[i] = idx;
        }
    }
}

void ServerStub::commit_locally(const Robot& robot, int cur_last) {
    {
        std::lock_guard<std::mutex> rl(records_mutex);
        customerRecords.UpdateRecord(robot.customer_id, robot.order_number);
    }

    {
        std::lock_guard<std::mutex> sl(state_mutex);
        server_state.committed_index = cur_last;
    }
}

void ServerStub::fulfill_promise(AdminRequest& req, int admin_id) {
    req.robot.admin_id = admin_id;
    req.promise.set_value(req.robot);
}

void ServerStub::HandleClientRequest(int client_fd, int engineer_id,
                                     AdminRequestQueue& adminQueue) {
    int identity = -1;

    if (!socket->ReceiveIdentification(identity, client_fd)) {
        close(client_fd);
        return;
    }

    if (identity == MSG_IDENTIFY_PFA) {
        handle_replication_request(client_fd);
        return;
    }

    RobotOrder request(0, 0, 0);
    while (running) {
        if (!ReceiveRequest(request, client_fd)) break;
        if (!process_request(request, client_fd, engineer_id, adminQueue))
            break;
    }

    close(client_fd);
}

bool ServerStub::process_request(const RobotOrder& request, int client_fd,
                                 int engineer_id,
                                 AdminRequestQueue& adminQueue) {
    if (request.request_type == 1)
        return handle_robot_order(request, engineer_id, client_fd, adminQueue);
    if (request.request_type == 2)
        return handle_record_read(request, client_fd);
    return false;
}

bool ServerStub::handle_robot_order(const RobotOrder& request, int engineer_id,
                                    int client_fd,
                                    AdminRequestQueue& adminQueue) {
    Robot robot(request.customer_id, request.order_number, request.request_type,
                engineer_id, -1);
    std::promise<Robot> p;
    std::future<Robot> fut = p.get_future();

    {
        std::lock_guard<std::mutex> lock(adminQueue.mtx);
        adminQueue.jobQueue.push(AdminRequest{robot, std::move(p)});
    }
    adminQueue.cv.notify_one();

    robot = fut.get();
    return ShipRobot(robot, client_fd);
}

bool ServerStub::handle_record_read(const RobotOrder& request, int client_fd) {
    CustomerRecord record;
    {
        std::lock_guard<std::mutex> rlock(records_mutex);
        record = customerRecords.GetRecord(request.customer_id);
    }
    return ReturnRecord(record, client_fd);
}

void ServerStub::handle_replication_request(int client_fd) {
    while (running) {
        ReplicationRequest req;
        if (!ReceiveReplicationRequest(req, client_fd)) {
            handle_pfa_disconnect();
            break;
        }

        log_replication_request(req);
        apply_replication_entry(req);
        apply_committed_entry(req);

        if (!send_replication_ack(client_fd)) break;
    }
    close(client_fd);
}

void ServerStub::handle_pfa_disconnect() {
    std::cout << "[IFA] Primary disconnected. Setting primary_id to -1."
              << std::endl;
    std::lock_guard<std::mutex> sl(state_mutex);
    server_state.primary_id = -1;
}

void ServerStub::handle_ifa_disconnect(int peer_index) {
    std::cout << "[PFA] Peer " << peer_index << " disconnected." << std::endl;
    std::lock_guard<std::mutex> sl(state_mutex);
    peer_alive[peer_index] = false;
    socket->SetPeerFd(peer_index, -1);
}

void ServerStub::log_replication_request(const ReplicationRequest& req) {
    std::cout << "[IFA] Replication: factory_id=" << req.factory_id
              << " last_index=" << req.last_index
              << " committed_index=" << req.committed_index << std::endl;
}

void ServerStub::apply_replication_entry(const ReplicationRequest& req) {
    std::lock_guard<std::mutex> sl(state_mutex);
    server_state.primary_id = req.factory_id;
    smr_log.WriteOperation(req.last_index, req.operation.op_code,
                           req.operation.arg1, req.operation.arg2);
    server_state.last_index = req.last_index;
}

void ServerStub::apply_committed_entry(const ReplicationRequest& req) {
    int current_commit;
    {
        std::lock_guard<std::mutex> sl(state_mutex);
        current_commit = server_state.committed_index;
    }

    if (req.committed_index <= current_commit) return;

    MapOp committed_op;
    {
        std::lock_guard<std::mutex> sl(state_mutex);
        committed_op = smr_log.GetOperation(req.committed_index);
    }

    {
        std::lock_guard<std::mutex> rl(records_mutex);
        customerRecords.UpdateRecord(committed_op.arg1, committed_op.arg2);
    }

    {
        std::lock_guard<std::mutex> sl(state_mutex);
        server_state.committed_index = req.committed_index;
    }
}

bool ServerStub::send_replication_ack(int client_fd) {
    return SendReplicationResponse(client_fd);
}

// -------------------- Peer Catchup --------------------
bool ServerStub::try_reconnect_and_catchup(int peer_index) {
    std::cout << "[PFA] Attempting reconnect to peer " << peer_index
              << std::endl;

    if (!socket->ReconnectPeer(peer_index, config.peers[peer_index])) {
        std::cerr << "[PFA] Reconnect failed for peer " << peer_index
                  << std::endl;
        return false;
    }

    int last;
    int factory_id;
    {
        std::lock_guard<std::mutex> sl(state_mutex);
        last = server_state.last_index;
        factory_id = server_state.factory_id;
    }

    std::cout << "[PFA] Sending " << last << " catchup entries to peer "
              << peer_index << std::endl;

    int start = peer_last_index[peer_index] + 1;
    for (int k = start; k <= last; k++) {
        MapOp op;
        {
            std::lock_guard<std::mutex> sl(state_mutex);
            op = smr_log.GetOperation(k);
        }

        ReplicationRequest catchup;
        catchup.factory_id = factory_id;
        catchup.last_index = k;
        catchup.committed_index = k;
        catchup.operation = op;

        if (!SendReplicationRequest(catchup, peer_index)) {
            handle_ifa_disconnect(peer_index);
            return false;
        }
        if (!ReceiveReplicationResponse(peer_index)) {
            handle_ifa_disconnect(peer_index);
            return false;
        }
    }

    peer_last_index[peer_index] = last;

    {
        std::lock_guard<std::mutex> sl(state_mutex);
        peer_alive[peer_index] = true;
    }

    std::cout << "[PFA] Peer " << peer_index << " caught up successfully."
              << std::endl;
    return true;
}

bool ServerStub::ReceiveRequest(RobotOrder& request, int client_fd) {
    return socket && socket->Receive(request, client_fd);
}
bool ServerStub::ShipRobot(const Robot& robot, int client_fd) {
    return socket && socket->Send(robot, client_fd);
}
bool ServerStub::ReturnRecord(const CustomerRecord& record, int client_fd) {
    return socket && socket->Send(record, client_fd);
}
bool ServerStub::SendReplicationRequest(const ReplicationRequest& request,
                                        int peer_index) {
    return socket && socket->SendReplicationRequest(request, peer_index);
}
bool ServerStub::ReceiveReplicationRequest(ReplicationRequest& request,
                                           int client_fd) {
    return socket && socket->ReceiveReplicationRequest(request, client_fd);
}
bool ServerStub::SendReplicationResponse(int client_fd) {
    return socket && socket->SendAck(client_fd);
}
bool ServerStub::ReceiveReplicationResponse(int peer_index) {
    return socket && socket->ReceiveAckFromPeer(peer_index);
}