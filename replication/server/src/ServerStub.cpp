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
    std::cout << "[PFA " << admin_id << "] Thread started (factory_id="
              << config.factory_id << ")." << std::endl;

    while (true) {
        // Wait for the next robot-order request from an engineer thread.
        std::unique_lock<std::mutex> lock(adminQueue.mtx);
        adminQueue.cv.wait(lock,
                           [&] { return !adminQueue.jobQueue.empty(); });
        AdminRequest req = std::move(adminQueue.jobQueue.front());
        adminQueue.jobQueue.pop();
        lock.unlock();

        std::cout << "[PFA " << admin_id
                  << "] Processing customer_id=" << req.robot.customer_id
                  << " order_number=" << req.robot.order_number << std::endl;

        // Step 2: On first request, become primary and connect to peers.
        {
            std::lock_guard<std::mutex> sl(state_mutex);
            if (server_state.primary_id != server_state.factory_id) {
                server_state.primary_id = server_state.factory_id;
                if (!peers_connected && !config.peers.empty()) {
                    std::cout << "[PFA] Connecting to "
                              << config.peers.size() << " peer(s)..."
                              << std::endl;
                    if (socket->connect_to_peers(config.peers)) {
                        peers_connected = true;
                        std::cout << "[PFA] Connected to all peers."
                                  << std::endl;
                    } else {
                        std::cerr << "[PFA] Failed to connect to peers."
                                  << std::endl;
                    }
                } else {
                    peers_connected = true;  // no peers → trivially done
                }
            }
        }

        // Step 3: Append MapOp to local log; advance last_index.
        int cur_last;
        int cur_committed;
        int factory_id;
        {
            std::lock_guard<std::mutex> sl(state_mutex);
            smr_log.add_operation(1, req.robot.customer_id,
                                  req.robot.order_number);
            server_state.last_index++;
            cur_last = server_state.last_index;
            cur_committed = server_state.committed_index;
            factory_id = server_state.factory_id;
        }

        // Step 4: Replicate to every backup.  For each peer:
        //   (a) send {factory_id, committed_index, last_index, MapOp}
        //   (b) wait for ack
        int num_peers = socket->num_peers();
        for (int i = 0; i < num_peers; i++) {
            ReplicationRequest rep;
            rep.factory_id = factory_id;
            rep.committed_index = cur_committed;
            rep.last_index = cur_last;
            rep.operation = {1, req.robot.customer_id,
                             req.robot.order_number};

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

        // Step 5: All backups have replicated — commit locally.
        {
            std::lock_guard<std::mutex> rl(records_mutex);
            customerRecords.update_record(req.robot.customer_id,
                                          req.robot.order_number);
        }
        {
            std::lock_guard<std::mutex> sl(state_mutex);
            server_state.committed_index = cur_last;
        }

        std::cout << "[PFA " << admin_id
                  << "] Committed index=" << cur_last << std::endl;

        // Fulfill the promise to unblock the engineer thread.
        req.robot.admin_id = admin_id;
        req.promise.set_value(req.robot);
    }
}

void ServerStub::handle_replication_request(int client_fd) {
    std::cout << "[IFA] Handler started for fd=" << client_fd << std::endl;

    while (true) {
        ReplicationRequest req;
        if (!ReceiveReplicationRequest(req, client_fd)) {
            std::cout << "[IFA] PFA disconnected (fd=" << client_fd << ")."
                      << std::endl;
            break;
        }

        std::cout << "[IFA] Replication: factory_id=" << req.factory_id
                  << " last_index=" << req.last_index
                  << " committed_index=" << req.committed_index << std::endl;

        // Step 4b-i/ii: record new primary; write MapOp at req.last_index.
        {
            std::lock_guard<std::mutex> sl(state_mutex);
            server_state.primary_id = req.factory_id;
            smr_log.write_operation(req.last_index, req.operation.op_code,
                                    req.operation.arg1, req.operation.arg2);
            server_state.last_index = req.last_index;
        }

        // Step 4b-iii: apply the committed entry to the customer record.
        if (req.committed_index > 0) {
            MapOp committed_op;
            {
                std::lock_guard<std::mutex> sl(state_mutex);
                committed_op = smr_log.get_operation(req.committed_index);
            }
            {
                std::lock_guard<std::mutex> rl(records_mutex);
                customerRecords.update_record(committed_op.arg1,
                                              committed_op.arg2);
            }
            {
                std::lock_guard<std::mutex> sl(state_mutex);
                server_state.committed_index = req.committed_index;
            }
            std::cout << "[IFA] Applied committed_index="
                      << req.committed_index << " customer_id="
                      << committed_op.arg1 << " order=" << committed_op.arg2
                      << std::endl;
        }

        // Step 4b-iv: send ack back to PFA.
        if (!SendReplicationResponse(client_fd)) {
            std::cerr << "[IFA] Failed to send ack." << std::endl;
            break;
        }
    }

    close(client_fd);
}

bool ServerStub::handle_robot_order(const RobotOrder& request, int engineer_id,
                                    int client_fd,
                                    AdminRequestQueue& adminQueue) {
    Robot robot(request.customer_id, request.order_number,
                request.request_type, engineer_id, -1);

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
    // --- Identification handshake (one int: 0=customer, 1=PFA) ---
    int identity = -1;
    if (!socket->receive_identification(identity, client_fd)) {
        std::cerr << "[Engineer " << engineer_id
                  << "] Failed to receive identification from fd=" << client_fd
                  << std::endl;
        close(client_fd);
        return;
    }

    if (identity == 1) {
        // PFA connected to this node → switch to IFA role.
        std::cout << "[Engineer " << engineer_id << "] fd=" << client_fd
                  << " is PFA — switching to IFA role." << std::endl;
        handle_replication_request(client_fd);
        return;
    }

    // identity == 0: regular customer, run engineer role.
    std::cout << "[Engineer " << engineer_id << "] fd=" << client_fd
              << " is a customer." << std::endl;

    RobotOrder request(0, 0, 0);
    while (true) {
        if (!ReceiveRequest(request, client_fd)) {
            std::cout << "[Engineer " << engineer_id << "] Client "
                      << client_fd << " disconnected." << std::endl;
            break;
        }

        std::cout << "[Engineer " << engineer_id
                  << "] request: customer_id=" << request.customer_id
                  << ", order_number=" << request.order_number
                  << ", type=" << request.request_type << std::endl;

        bool ok = false;
        if (request.request_type == 1) {
            ok = handle_robot_order(request, engineer_id, client_fd,
                                    adminQueue);
        } else if (request.request_type == 2) {
            ok = handle_record_read(request, engineer_id, client_fd);
        } else {
            std::cerr << "[Engineer " << engineer_id
                      << "] Unknown request_type=" << request.request_type
                      << std::endl;
            break;
        }

        if (!ok) {
            std::cerr << "[Engineer " << engineer_id
                      << "] Failed to send response." << std::endl;
            break;
        }
    }

    std::cout << "[Engineer " << engineer_id << "] Closing connection "
              << client_fd << std::endl;
    close(client_fd);
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
    return socket &&
           socket->receive_replication_request(request, client_fd);
}

bool ServerStub::SendReplicationResponse(int client_fd) {
    return socket && socket->send_ack(client_fd);
}

bool ServerStub::ReceiveReplicationResponse(int peer_index) {
    return socket && socket->receive_ack_from_peer(peer_index);
}

