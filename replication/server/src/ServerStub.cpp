#include "../include/ServerStub.h"
#include <unistd.h>
#include <chrono>
#include <iostream>
#include <thread>
#include <future>
#include <mutex>
#include <queue>

ServerStub::ServerStub() : socket(nullptr), peers_connected(false), running(true) {}

void ServerStub::init(ServerSocket* socket, const ServerConfig& config) {
    this->socket = socket;
    this->config = config;
    this->customerRecords = CustomerRecords();
    this->peers_connected = false;

    // Initialize server state
    this->server_state.factory_id = config.factory_id;
    this->server_state.primary_id = -1;
    this->server_state.last_index = 0;
    this->server_state.committed_index = 0;

    // Heartbeat tracking
    peer_last_heartbeat.resize(config.peers.size(), std::chrono::steady_clock::now());
    peer_alive.resize(config.peers.size(), true);

    // Start background threads
    start_heartbeat_sender();
    start_failure_detector();
}

void ServerStub::AdminProcessRequests(int admin_id, AdminRequestQueue& adminQueue) {
    std::cout << "[PFA " << admin_id << "] Admin thread started (factory_id=" << config.factory_id << ")" << std::endl;

    while (running) {
        AdminRequest req = wait_for_admin_request(adminQueue);

        ensure_primary_and_connect_peers();

        int cur_last = append_to_log(req.robot);
        replicate_to_peers(req.robot, cur_last);
        commit_locally(req.robot, cur_last);

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

    if (server_state.primary_id == server_state.factory_id) return;

    server_state.primary_id = server_state.factory_id;

    if (!peers_connected && !config.peers.empty()) {
        std::cout << "[PFA] Connecting to " << config.peers.size() << " peer(s)..." << std::endl;

        if (socket->connect_to_peers(config.peers)) {
            peers_connected = true;
            std::cout << "[PFA] Connected to all peers." << std::endl;
        } else {
            std::cerr << "[PFA] Failed to connect to peers." << std::endl;
        }
    } else {
        peers_connected = true;
    }
}

int ServerStub::append_to_log(const Robot& robot) {
    std::lock_guard<std::mutex> sl(state_mutex);
    smr_log.add_operation(1, robot.customer_id, robot.order_number);
    server_state.last_index++;
    return server_state.last_index;
}

void ServerStub::replicate_to_peers(const Robot& robot, int cur_last) {
    int factory_id;
    int committed_index;

    {
        std::lock_guard<std::mutex> sl(state_mutex);
        factory_id = server_state.factory_id;
        committed_index = server_state.committed_index;
    }

    for (int i = 0; i < socket->num_peers(); i++) {
        if (!peer_alive[i]) {
            if (!try_reconnect_and_catchup(i, factory_id, cur_last))
                continue;
        }

        ReplicationRequest rep;
        rep.factory_id = factory_id;
        rep.committed_index = committed_index;
        rep.last_index = cur_last;
        rep.operation = {1, robot.customer_id, robot.order_number};

        bool sent = false;
        for (int retry = 0; retry < 3 && !sent; retry++) {
            sent = SendReplicationRequest(rep, i);
            if (!sent) std::this_thread::sleep_for(std::chrono::milliseconds(200));
        }

        if (!sent) {
            std::cerr << "[PFA] Failed to send replication to peer " << i << std::endl;
            handle_ifa_disconnect(i);
            continue;
        }

        if (!ReceiveReplicationResponse(i)) {
            std::cerr << "[PFA] Failed to receive ack from peer " << i << std::endl;
        }
    }
}

void ServerStub::commit_locally(const Robot& robot, int cur_last) {
    {
        std::lock_guard<std::mutex> rl(records_mutex);
        customerRecords.update_record(robot.customer_id, robot.order_number);
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
    std::cout << "[IFA] Primary disconnected. Setting primary_id to -1." << std::endl;
    std::lock_guard<std::mutex> sl(state_mutex);
    server_state.primary_id = -1;
}

void ServerStub::handle_ifa_disconnect(int peer_index) {
    std::cout << "[IFA] Peer " << peer_index << " disconnected." << std::endl;
    {
        std::lock_guard<std::mutex> sl(state_mutex);
        peer_alive[peer_index] = false;
        socket->set_peer_fd(peer_index, -1);
    }
    elect_new_primary();
}

void ServerStub::elect_new_primary() {
    std::lock_guard<std::mutex> sl(state_mutex);

    if (server_state.primary_id == server_state.factory_id) return;

    for (size_t i = 0; i < config.peers.size(); i++) {
        if (peer_alive[i]) {
            server_state.primary_id = config.peers[i].id;
            std::cout << "[PFA] New primary elected: factory_id=" << server_state.primary_id << std::endl;
            return;
        }
    }

    server_state.primary_id = server_state.factory_id;
}

void ServerStub::start_heartbeat_sender() {
    std::thread([this]() {
        while (running) {
            for (int i = 0; i < socket->num_peers(); i++) {
                if (!peer_alive[i]) continue;

                if (!socket->send_heartbeat(i)) {
                    handle_ifa_disconnect(i);
                }
            }
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    }).detach();
}

void ServerStub::handle_heartbeat(int peer_index) {
    std::lock_guard<std::mutex> lock(heartbeat_mutex);
    peer_last_heartbeat[peer_index] = std::chrono::steady_clock::now();
    peer_alive[peer_index] = true;
}

void ServerStub::start_failure_detector() {
    std::thread([this]() {
        const auto timeout = std::chrono::seconds(3);

        while (running) {
            auto now = std::chrono::steady_clock::now();

            for (size_t i = 0; i < peer_last_heartbeat.size(); i++) {
                std::lock_guard<std::mutex> lock(heartbeat_mutex);

                if (!peer_alive[i]) continue;

                auto diff = std::chrono::duration_cast<std::chrono::seconds>(now - peer_last_heartbeat[i]);
                if (diff > timeout) {
                    peer_alive[i] = false;
                    handle_ifa_disconnect(i);
                }
            }

            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    }).detach();
}

bool ServerStub::handle_robot_order(const RobotOrder& request, int engineer_id, int client_fd, AdminRequestQueue& adminQueue) {
    Robot robot(request.customer_id, request.order_number, request.request_type, engineer_id, -1);
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
        record = customerRecords.get_record(request.customer_id);
    }
    return ReturnRecord(record, client_fd);
}

void ServerStub::HandleClientRequest(int client_fd, int engineer_id, AdminRequestQueue& adminQueue) {
    int identity = -1;

    if (!socket->receive_identification(identity, client_fd)) {
        close(client_fd);
        return;
    }

    if (identity == 1) {
        handle_replication_request(client_fd);
        return;
    }

    RobotOrder request(0, 0, 0);
    while (running) {
        if (!ReceiveRequest(request, client_fd)) break;
        if (!process_request(request, client_fd, engineer_id, adminQueue)) break;
    }

    close(client_fd);
}

bool ServerStub::process_request(const RobotOrder& request, int client_fd, int engineer_id, AdminRequestQueue& adminQueue) {
    if (request.request_type == 1) return handle_robot_order(request, engineer_id, client_fd, adminQueue);
    if (request.request_type == 2) return handle_record_read(request, client_fd);

    return false;
}

bool ServerStub::ReceiveRequest(RobotOrder& request, int client_fd) { return socket && socket->receive(request, client_fd); }
bool ServerStub::ShipRobot(const Robot& robot, int client_fd) { return socket && socket->send(robot, client_fd); }
bool ServerStub::ReturnRecord(const CustomerRecord& record, int client_fd) { return socket && socket->send(record, client_fd); }
bool ServerStub::SendReplicationRequest(const ReplicationRequest& request, int peer_index) { return socket && socket->send_replication_request(request, peer_index); }
bool ServerStub::ReceiveReplicationRequest(ReplicationRequest& request, int client_fd) { return socket && socket->receive_replication_request(request, client_fd); }
bool ServerStub::SendReplicationResponse(int client_fd) { return socket && socket->send_ack(client_fd); }
bool ServerStub::ReceiveReplicationResponse(int peer_index) { return socket && socket->receive_ack_from_peer(peer_index); }

void ServerStub::log_replication_request(const ReplicationRequest& req) {
    std::cout << "[IFA] Replication: factory_id=" << req.factory_id
              << " last_index=" << req.last_index
              << " committed_index=" << req.committed_index << std::endl;
}

void ServerStub::apply_replication_entry(const ReplicationRequest& req) {
    std::lock_guard<std::mutex> sl(state_mutex);
    server_state.primary_id = req.factory_id;
    smr_log.write_operation(req.last_index, req.operation.op_code, req.operation.arg1, req.operation.arg2);
    server_state.last_index = req.last_index;
}

void ServerStub::apply_committed_entry(const ReplicationRequest& req) {
    if (req.committed_index <= 0) return;

    MapOp committed_op;
    {
        std::lock_guard<std::mutex> sl(state_mutex);
        committed_op = smr_log.get_operation(req.committed_index);
    }

    {
        std::lock_guard<std::mutex> rl(records_mutex);
        customerRecords.update_record(committed_op.arg1, committed_op.arg2);
    }

    {
        std::lock_guard<std::mutex> sl(state_mutex);
        server_state.committed_index = req.committed_index;
    }
}

bool ServerStub::send_replication_ack(int client_fd) {
    if (!SendReplicationResponse(client_fd)) return false;
    return true;
}

bool ServerStub::try_reconnect_and_catchup(int peer_index, int factory_id,
                                            int cur_last) {
    std::cout << "[PFA] Peer " << peer_index
              << " is down. Attempting reconnect..." << std::endl;

    if (!socket->reconnect_peer(peer_index, config.peers[peer_index])) {
        std::cerr << "[PFA] Reconnect to peer " << peer_index
                  << " failed." << std::endl;
        return false;
    }

    // Send every committed log entry one by one so the repaired server
    // catches up to the same state as the rest of the cluster.
    std::cout << "[PFA] Sending " << (cur_last - 1)
              << " catchup entries to peer " << peer_index << std::endl;

    for (int k = 1; k < cur_last; k++) {
        MapOp op;
        {
            std::lock_guard<std::mutex> sl(state_mutex);
            op = smr_log.get_operation(k);
        }

        ReplicationRequest catchup;
        catchup.factory_id = factory_id;
        catchup.last_index = k;
        catchup.committed_index = k;
        catchup.operation = op;

        if (!SendReplicationRequest(catchup, peer_index)) {
            std::cerr << "[PFA] Catchup send failed at entry " << k
                      << " for peer " << peer_index << std::endl;
            handle_ifa_disconnect(peer_index);
            return false;
        }

        if (!ReceiveReplicationResponse(peer_index)) {
            std::cerr << "[PFA] No ack for catchup entry " << k
                      << " from peer " << peer_index << std::endl;
            handle_ifa_disconnect(peer_index);
            return false;
        }
    }

    // reset the heartbeat timestamp before marking the peer alive so the
    // failure detector does not immediately expire the peer again.
    {
        std::lock_guard<std::mutex> hlock(heartbeat_mutex);
        peer_last_heartbeat[peer_index] = std::chrono::steady_clock::now();
    }
    {
        std::lock_guard<std::mutex> sl(state_mutex);
        peer_alive[peer_index] = true;
    }

    std::cout << "[PFA] Peer " << peer_index
              << " is back online and fully caught up." << std::endl;
    return true;
}