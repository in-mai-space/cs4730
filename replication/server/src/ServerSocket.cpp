#include "../include/ServerSocket.h"

#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cstring>
#include <iostream>

#include "../../common/include/CustomerRecords.h"
#include "../../common/include/Robot.h"
#include "../../common/include/RobotOrder.h"

ServerSocket::ServerSocket() : socket_fd(-1), client_fd(-1) {}

bool ServerSocket::listen(int port) {
    socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd < 0) {
        std::cerr << "Error creating socket" << std::endl;
        return false;
    }

    int opt = 1;
    if (setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        std::cerr << "Error setting socket options" << std::endl;
        close(socket_fd);
        return false;
    }

    sockaddr_in serv_addr{};
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(port);

    if (bind(socket_fd, (sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        std::cerr << "Error binding socket" << std::endl;
        close(socket_fd);
        return false;
    }

    if (::listen(socket_fd, 10) < 0) {
        std::cerr << "Error listening on socket" << std::endl;
        close(socket_fd);
        return false;
    }

    std::cout << "Server listening on port " << port << std::endl;
    return true;
}

int ServerSocket::accept() {
    sockaddr_in client_addr{};
    socklen_t client_len = sizeof(client_addr);

    int new_client_fd =
        ::accept(socket_fd, (sockaddr*)&client_addr, &client_len);

    if (new_client_fd < 0) {
        std::cerr << "Error accepting client connection" << std::endl;
        return -1;
    }

    return new_client_fd;
}

bool ServerSocket::connect_to_peers(const std::vector<PeerInfo>& peers) {
    for (const auto& peer : peers) {

        int peer_fd = socket(AF_INET, SOCK_STREAM, 0);
        if (peer_fd < 0) {
            std::cerr << "Failed to create peer socket\n";
            return false;
        }

        sockaddr_in peer_addr{};
        peer_addr.sin_family = AF_INET;
        peer_addr.sin_port = htons(peer.port);

        if (inet_pton(AF_INET, peer.ip.c_str(), &peer_addr.sin_addr) <= 0) {
            std::cerr << "Invalid peer address\n";
            close(peer_fd);
            return false;
        }

        if (connect(peer_fd, (sockaddr*)&peer_addr, sizeof(peer_addr)) < 0) {
            std::cerr << "Failed to connect to peer\n";
            close(peer_fd);
            return false;
        }

        peer_fds.push_back(peer_fd);
    }

    return true;
}

bool ServerSocket::send(const Robot& robot, int client_fd) {
    int fd = (client_fd != -1) ? client_fd : this->client_fd;

    char buffer[5 * sizeof(int)];
    int len = marshall(robot, buffer, sizeof(buffer));
    if (len <= 0) return false;

    return send_all(buffer, len, fd);
}

bool ServerSocket::receive(RobotOrder& order, int client_fd) {
    int fd = (client_fd != -1) ? client_fd : this->client_fd;

    char buffer[3 * sizeof(int)];
    if (!receive_all(buffer, sizeof(buffer), fd)) return false;

    return unmarshall(buffer, sizeof(buffer), order) > 0;
}

bool ServerSocket::send(const CustomerRecord& record, int client_fd) {
    int fd = (client_fd != -1) ? client_fd : this->client_fd;

    char buffer[2 * sizeof(int)];
    int len = marshall(record, buffer, sizeof(buffer));
    if (len <= 0) return false;

    return send_all(buffer, len, fd);
}

bool ServerSocket::send_replication_request(
    const ReplicationRequest& request) {

    int fd = this->client_fd;
    if (fd < 0) return false;

    char buffer[6 * sizeof(int)];
    int len = marshall(request, buffer, sizeof(buffer));
    if (len <= 0) return false;

    return send_all(buffer, len, fd);
}

bool ServerSocket::receive_replication_request(
    ReplicationRequest& request) {

    int fd = this->client_fd;
    if (fd < 0) return false;

    char buffer[6 * sizeof(int)];
    if (!receive_all(buffer, sizeof(buffer), fd))
        return false;

    return unmarshall(buffer, sizeof(buffer), request) > 0;
}

int ServerSocket::marshall(const Robot& robot, char* buffer, int buffer_size) {
    if (buffer_size < 5 * (int)sizeof(int)) return -1;

    int net_customer_id = htonl(robot.customer_id);
    int net_order_number = htonl(robot.order_number);
    int net_request_type = htonl(robot.request_type);
    int net_engineer_id = htonl(robot.engineer_id);
    int net_admin_id = htonl(robot.admin_id);

    std::memcpy(buffer, &net_customer_id, sizeof(int));
    std::memcpy(buffer + sizeof(int), &net_order_number, sizeof(int));
    std::memcpy(buffer + 2*sizeof(int), &net_request_type, sizeof(int));
    std::memcpy(buffer + 3*sizeof(int), &net_engineer_id, sizeof(int));
    std::memcpy(buffer + 4*sizeof(int), &net_admin_id, sizeof(int));

    return 5 * sizeof(int);
}

int ServerSocket::unmarshall(const char* buffer, int buffer_size,
                             RobotOrder& order) {
    if (buffer_size < 3 * (int)sizeof(int)) return -1;

    int net_customer_id = 0;
    int net_order_number = 0;
    int net_request_type = 0;

    std::memcpy(&net_customer_id, buffer, sizeof(int));
    std::memcpy(&net_order_number, buffer + sizeof(int), sizeof(int));
    std::memcpy(&net_request_type, buffer + 2*sizeof(int), sizeof(int));

    order.customer_id = ntohl(net_customer_id);
    order.order_number = ntohl(net_order_number);
    order.request_type = ntohl(net_request_type);

    return 3 * sizeof(int);
}

int ServerSocket::marshall(const CustomerRecord& record,
                           char* buffer,
                           int buffer_size) {
    if (buffer_size < 2 * (int)sizeof(int)) return -1;

    int net_customer_id = htonl(record.customer_id);
    int net_last_order = htonl(record.last_order);

    std::memcpy(buffer, &net_customer_id, sizeof(int));
    std::memcpy(buffer + sizeof(int), &net_last_order, sizeof(int));

    return 2 * sizeof(int);
}

int ServerSocket::marshall(const ReplicationRequest& request,
                           char* buffer,
                           int buffer_size) {
    if (buffer_size < 6 * (int)sizeof(int)) return -1;

    int net_factory_id      = htonl(request.factory_id);
    int net_committed_index = htonl(request.committed_index);
    int net_last_index      = htonl(request.last_index);

    int net_opcode = htonl(request.operation.op_code);
    int net_arg1   = htonl(request.operation.arg1);
    int net_arg2   = htonl(request.operation.arg2);

    std::memcpy(buffer, &net_factory_id, sizeof(int));
    std::memcpy(buffer + 1*sizeof(int), &net_committed_index, sizeof(int));
    std::memcpy(buffer + 2*sizeof(int), &net_last_index, sizeof(int));
    std::memcpy(buffer + 3*sizeof(int), &net_opcode, sizeof(int));
    std::memcpy(buffer + 4*sizeof(int), &net_arg1, sizeof(int));
    std::memcpy(buffer + 5*sizeof(int), &net_arg2, sizeof(int));

    return 6 * sizeof(int);
}

int ServerSocket::unmarshall(const char* buffer,
                             int buffer_size,
                             ReplicationRequest& request) {
    if (buffer_size < 6 * (int)sizeof(int)) return -1;

    int net_factory_id = 0;
    int net_committed_index = 0;
    int net_last_index = 0;
    int net_opcode = 0;
    int net_arg1 = 0;
    int net_arg2 = 0;

    std::memcpy(&net_factory_id, buffer, sizeof(int));
    std::memcpy(&net_committed_index, buffer + 1*sizeof(int), sizeof(int));
    std::memcpy(&net_last_index, buffer + 2*sizeof(int), sizeof(int));
    std::memcpy(&net_opcode, buffer + 3*sizeof(int), sizeof(int));
    std::memcpy(&net_arg1, buffer + 4*sizeof(int), sizeof(int));
    std::memcpy(&net_arg2, buffer + 5*sizeof(int), sizeof(int));

    request.factory_id = ntohl(net_factory_id);
    request.committed_index = ntohl(net_committed_index);
    request.last_index = ntohl(net_last_index);
    request.operation.op_code = ntohl(net_opcode);
    request.operation.arg1 = ntohl(net_arg1);
    request.operation.arg2 = ntohl(net_arg2);

    return 6 * sizeof(int);
}

bool ServerSocket::send_all(const char* data, size_t len, int client_fd) {
    size_t total = 0;
    while (total < len) {
        ssize_t sent = ::send(client_fd, data + total, len - total, 0);
        if (sent <= 0) return false;
        total += sent;
    }
    return true;
}

bool ServerSocket::receive_all(char* data, size_t len, int client_fd) {
    size_t total = 0;
    while (total < len) {
        ssize_t recvd = ::recv(client_fd, data + total, len - total, 0);
        if (recvd <= 0) return false;
        total += recvd;
    }
    return true;
}