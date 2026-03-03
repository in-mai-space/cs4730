#include "../include/ServerStub.h"

#include <unistd.h>

#include <iostream>

#include "../../common/include/CustomerRecords.h"
#include "../../common/include/Robot.h"
#include "../../common/include/RobotOrder.h"

ServerStub::ServerStub() : socket(nullptr), peers_connected(false) {}

void ServerStub::init(ServerSocket* socket, const ServerConfig& config) {
    this->socket = socket;
    this->config = config;
    this->customerRecords = CustomerRecords();
    this->peers_connected = false;
    this->server_state.factory_id = config.factory_id;
    this->server_state.primary_id = -1;
    this->server_state.last_index = 0;
    this->server_state.committed_index = 0;
}

void ServerStub::admin_process_requests(int admin_id,
                                        AdminRequestQueue& adminQueue) {
    std::cout << "[PFA " << admin_id
              << "] Thread started (factory_id=" << config.factory_id << ")."
              << std::endl;

    while (true) {
        AdminRequest req = wait_for_admin_request(adminQueue);

        std::cout << "[PFA " << admin_id
                  << "] Processing customer_id=" << req.robot.customer_id
                  << " order_number=" << req.robot.order_number << std::endl;

        ensure_primary_and_connect_peers();

        int cur_last = append_to_log(req.robot);

        replicate_to_peers(req.robot, cur_last);

        commit_locally(req.robot, cur_last);

        std::cout << "[PFA " << admin_id << "] Committed index=" << cur_last
                  << std::endl;

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
        std::cout << "[PFA] Connecting to " << config.peers.size()
                  << " peer(s)..." << std::endl;

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

    int num_peers = socket->num_peers();

    for (int i = 0; i < num_peers; i++) {
        ReplicationRequest rep;
        rep.factory_id = factory_id;
        rep.committed_index = committed_index;
        rep.last_index = cur_last;
        rep.operation = {1, robot.customer_id, robot.order_number};

        if (!SendReplicationRequest(rep, i)) {
            std::cerr << "[PFA] Failed to send replication to peer " << i
                      << std::endl;
            continue;
        }

        if (!ReceiveReplicationResponse(i)) {
            std::cerr << "[PFA] Failed to receive ack from peer " << i
                      << std::endl;
        } else {
            std::cout << "[PFA] Peer " << i << " acked." << std::endl;
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
    std::cout << "[IFA] Handler started for fd=" << client_fd << std::endl;

    while (true) {
        ReplicationRequest req;

        if (!ReceiveReplicationRequest(req, client_fd)) {
            handle_pfa_disconnect(client_fd);
            break;
        }

        log_replication_request(req);

        apply_replication_entry(req);

        apply_committed_entry(req);

        if (!send_replication_ack(client_fd)) break;
    }

    close(client_fd);
}

void ServerStub::handle_pfa_disconnect(int client_fd) {
    std::cout << "[IFA] PFA disconnected (fd=" << client_fd << ")."
              << std::endl;
}

void ServerStub::log_replication_request(const ReplicationRequest& req) {
    std::cout << "[IFA] Replication: factory_id=" << req.factory_id
              << " last_index=" << req.last_index
              << " committed_index=" << req.committed_index << std::endl;
}

void ServerStub::apply_replication_entry(const ReplicationRequest& req) {
    std::lock_guard<std::mutex> sl(state_mutex);

    server_state.primary_id = req.factory_id;

    smr_log.write_operation(req.last_index, req.operation.op_code,
                            req.operation.arg1, req.operation.arg2);

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
    if (!SendReplicationResponse(client_fd)) {
        std::cerr << "[IFA] Failed to send ack." << std::endl;
        return false;
    }
    return true;
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

    std::cout << "[Engineer " << engineer_id
              << "] Shipping robot (admin_id=" << robot.admin_id << ")"
              << std::endl;

    return ShipRobot(robot, client_fd);
}

bool ServerStub::handle_record_read(const RobotOrder& request, int engineer_id,
                                    int client_fd) {
    CustomerRecord record;

    {
        std::lock_guard<std::mutex> rlock(records_mutex);
        record = customerRecords.get_record(request.customer_id);
    }

    std::cout << "[Engineer " << engineer_id
              << "] Returning record: customer_id=" << record.customer_id
              << ", last_order=" << record.last_order << std::endl;

    return ReturnRecord(record, client_fd);
}

void ServerStub::handle_client_request(int client_fd, int engineer_id,
                                       AdminRequestQueue& adminQueue) {
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

    while (true) {
        if (!ReceiveRequest(request, client_fd)) break;

        if (!process_request(request, client_fd, engineer_id, adminQueue))
            break;
    }

    close(client_fd);
}

bool ServerStub::process_request(const RobotOrder& request, int client_fd,
                                 int engineer_id,
                                 AdminRequestQueue& adminQueue) {
    if (request.request_type == 1) {
        return handle_robot_order(request, engineer_id, client_fd, adminQueue);
    }

    if (request.request_type == 2) {
        return handle_record_read(request, engineer_id, client_fd);
    }

    std::cerr << "[Engineer " << engineer_id
              << "] Unknown request_type=" << request.request_type << std::endl;

    return false;
}

bool ServerStub::ReceiveRequest(RobotOrder& request, int client_fd) {
    return socket && socket->receive(request, client_fd);
}

bool ServerStub::ShipRobot(const Robot& robot, int client_fd) {
    return socket && socket->send(robot, client_fd);
}

bool ServerStub::ReturnRecord(const CustomerRecord& record, int client_fd) {
    return socket && socket->send(record, client_fd);
}

bool ServerStub::SendReplicationRequest(const ReplicationRequest& request,
                                        int peer_index) {
    return socket && socket->send_replication_request(request, peer_index);
}

bool ServerStub::ReceiveReplicationRequest(ReplicationRequest& request,
                                           int client_fd) {
    return socket && socket->receive_replication_request(request, client_fd);
}

bool ServerStub::SendReplicationResponse(int client_fd) {
    return socket && socket->send_ack(client_fd);
}

bool ServerStub::ReceiveReplicationResponse(int peer_index) {
    return socket && socket->receive_ack_from_peer(peer_index);
}