#include "../include/ServerSocket.h"

#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cstring>
#include <iostream>

enum MessageType {
    MSG_CLIENT_REQUEST = 0,
    MSG_SERVER_ACK = 1,
    MSG_IDENTIFY_PFA = 2,
    MSG_HEARTBEAT = 3,
    MSG_REPLICATION_REQUEST = 4
};

ServerSocket::ServerSocket() : socket_fd(-1), client_fd(-1) {}

bool ServerSocket::listen(int port) {
    socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd < 0) return false;

    int opt = 1;
    setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);

    if (bind(socket_fd, (sockaddr*)&addr, sizeof(addr)) < 0) return false;

    if (::listen(socket_fd, 20) < 0) return false;

    std::cout << "Server listening on port " << port << std::endl;

    return true;
}

int ServerSocket::accept() {
    sockaddr_in client_addr{};
    socklen_t len = sizeof(client_addr);

    int fd = ::accept(socket_fd, (sockaddr*)&client_addr, &len);

    if (fd < 0) return -1;

    return fd;
}

bool ServerSocket::connect_to_peers(const std::vector<PeerInfo>& peers) {
    peer_fds.assign(peers.size(), -1);

    for (size_t i = 0; i < peers.size(); i++) {
        const auto& peer = peers[i];

        int fd = ::socket(AF_INET, SOCK_STREAM, 0);
        if (fd < 0) continue;

        sockaddr_in addr{};
        addr.sin_family = AF_INET;
        addr.sin_port = htons(peer.port);

        if (inet_pton(AF_INET, peer.ip.c_str(), &addr.sin_addr) <= 0) {
            close(fd);
            continue;
        }

        if (::connect(fd, (sockaddr*)&addr, sizeof(addr)) < 0) {
            close(fd);
            continue;
        }

        identify_as_server(fd);
        peer_fds[i] = fd;

        std::cout << "[PFA] Connected peer " << peer.id << std::endl;
    }

    return true;
}

bool ServerSocket::is_peer_connected(int index) const {
    if (index < 0 || index >= (int)peer_fds.size()) return false;
    return peer_fds[index] >= 0;
}

bool ServerSocket::reconnect_peer(int index, const PeerInfo& peer) {
    if (index < 0 || index >= (int)peer_fds.size()) return false;

    // Close the stale file descriptor if it is still open.
    if (peer_fds[index] >= 0) {
        close(peer_fds[index]);
        peer_fds[index] = -1;
    }

    int fd = ::socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) return false;

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(peer.port);

    if (inet_pton(AF_INET, peer.ip.c_str(), &addr.sin_addr) <= 0) {
        close(fd);
        return false;
    }

    if (::connect(fd, (sockaddr*)&addr, sizeof(addr)) < 0) {
        close(fd);
        return false;
    }

    identify_as_server(fd);
    peer_fds[index] = fd;

    std::cout << "[PFA] Reconnected to peer " << peer.id << std::endl;
    return true;
}

bool ServerSocket::identify_as_server(int client_fd) {
    int type = htonl(MSG_IDENTIFY_PFA);

    return send_all((char*)&type, sizeof(int), client_fd);
}

bool ServerSocket::send_ack(int client_fd) {
    int ack = htonl(MSG_SERVER_ACK);

    return send_all((char*)&ack, sizeof(int), client_fd);
}

bool ServerSocket::receive_ack(int client_fd) {
    int val = 0;

    if (!receive_all((char*)&val, sizeof(int), client_fd)) return false;

    return ntohl(val) == MSG_SERVER_ACK;
}

bool ServerSocket::receive_ack_from_peer(int peer_index) {
    if (peer_index < 0 || peer_index >= (int)peer_fds.size()) return false;

    return receive_ack(peer_fds[peer_index]);
}

void ServerSocket::set_peer_fd(int index, int fd) {
    if (index < 0 || index >= (int)peer_fds.size()) return;

    if (peer_fds[index] >= 0 && fd < 0) close(peer_fds[index]);

    peer_fds[index] = fd;
}

bool ServerSocket::receive_identification(int& type, int client_fd) {
    int net_type = 0;

    if (!receive_all((char*)&net_type, sizeof(int), client_fd)) return false;

    type = ntohl(net_type);

    return true;
}

bool ServerSocket::send_heartbeat(int peer_index) {
    if (peer_index < 0 || peer_index >= (int)peer_fds.size()) return false;

    int fd = peer_fds[peer_index];

    int type = htonl(MSG_HEARTBEAT);

    return send_all((char*)&type, sizeof(int), fd);
}

bool ServerSocket::send(const Robot& robot, int client_fd) {
    int fd = (client_fd != -1) ? client_fd : this->client_fd;

    char buffer[5 * sizeof(int)];

    int len = marshall(robot, buffer, sizeof(buffer));

    if (len <= 0) return false;

    return send_all(buffer, len, fd);
}

bool ServerSocket::send(const CustomerRecord& record, int client_fd) {
    int fd = (client_fd != -1) ? client_fd : this->client_fd;

    char buffer[2 * sizeof(int)];

    int len = marshall(record, buffer, sizeof(buffer));

    if (len <= 0) return false;

    return send_all(buffer, len, fd);
}

bool ServerSocket::receive(RobotOrder& order, int client_fd) {
    int fd = client_fd;

    char buffer[3 * sizeof(int)];

    if (!receive_all(buffer, sizeof(buffer), fd)) return false;

    return unmarshall(buffer, sizeof(buffer), order) > 0;
}

bool ServerSocket::send_replication_request(const ReplicationRequest& req,
                                            int peer_index) {
    if (peer_index < 0 || peer_index >= (int)peer_fds.size()) return false;

    char buffer[6 * sizeof(int)];

    int len = marshall(req, buffer, sizeof(buffer));

    if (len <= 0) return false;

    return send_all(buffer, len, peer_fds[peer_index]);
}

bool ServerSocket::receive_replication_request(ReplicationRequest& req,
                                               int client_fd) {
    char buffer[6 * sizeof(int)];

    if (!receive_all(buffer, sizeof(buffer), client_fd)) return false;

    return unmarshall(buffer, sizeof(buffer), req) > 0;
}

bool ServerSocket::send_all(const char* data, size_t len, int fd) {
    if (!data || fd < 0) return false;

    size_t total = 0;

    while (total < len) {
        ssize_t sent = ::send(fd, data + total, len - total, 0);

        if (sent <= 0) return false;

        total += sent;
    }

    return true;
}

bool ServerSocket::receive_all(char* data, size_t len, int fd) {
    if (!data || fd < 0) return false;

    size_t total = 0;

    while (total < len) {
        ssize_t recvd = ::recv(fd, data + total, len - total, 0);

        if (recvd <= 0) return false;

        total += recvd;
    }

    return true;
}

int ServerSocket::marshall(const Robot& robot, char* buffer, int size) {
    if (size < 5 * (int)sizeof(int)) return -1;

    int net_customer_id = htonl(robot.customer_id);
    int net_order_number = htonl(robot.order_number);
    int net_request_type = htonl(robot.request_type);
    int net_engineer_id = htonl(robot.engineer_id);
    int net_admin_id = htonl(robot.admin_id);

    int offset = 0;

    memcpy(buffer + offset, &net_customer_id, sizeof(int));
    offset += sizeof(int);

    memcpy(buffer + offset, &net_order_number, sizeof(int));
    offset += sizeof(int);

    memcpy(buffer + offset, &net_request_type, sizeof(int));
    offset += sizeof(int);

    memcpy(buffer + offset, &net_engineer_id, sizeof(int));
    offset += sizeof(int);

    memcpy(buffer + offset, &net_admin_id, sizeof(int));

    return 5 * sizeof(int);
}

int ServerSocket::marshall(const CustomerRecord& record, char* buffer,
                           int size) {
    if (size < 2 * (int)sizeof(int)) return -1;

    int net_customer_id = htonl(record.customer_id);
    int net_last_order = htonl(record.last_order);

    memcpy(buffer, &net_customer_id, sizeof(int));
    memcpy(buffer + sizeof(int), &net_last_order, sizeof(int));

    return 2 * sizeof(int);
}

int ServerSocket::marshall(const ReplicationRequest& req, char* buffer,
                           int size) {
    if (size < 6 * (int)sizeof(int)) return -1;

    int vals[6];

    vals[0] = htonl(req.factory_id);
    vals[1] = htonl(req.committed_index);
    vals[2] = htonl(req.last_index);
    vals[3] = htonl(req.operation.op_code);
    vals[4] = htonl(req.operation.arg1);
    vals[5] = htonl(req.operation.arg2);

    memcpy(buffer, vals, 6 * sizeof(int));

    return 6 * sizeof(int);
}

int ServerSocket::unmarshall(const char* buffer, int size, RobotOrder& order) {
    if (size < 3 * (int)sizeof(int)) return -1;

    int vals[3];

    memcpy(vals, buffer, 3 * sizeof(int));

    order.customer_id = ntohl(vals[0]);
    order.order_number = ntohl(vals[1]);
    order.request_type = ntohl(vals[2]);

    return 3 * sizeof(int);
}

int ServerSocket::unmarshall(const char* buffer, int size,
                             ReplicationRequest& req) {
    if (size < 6 * (int)sizeof(int)) return -1;

    int vals[6];

    memcpy(vals, buffer, 6 * sizeof(int));

    req.factory_id = ntohl(vals[0]);
    req.committed_index = ntohl(vals[1]);
    req.last_index = ntohl(vals[2]);
    req.operation.op_code = ntohl(vals[3]);
    req.operation.arg1 = ntohl(vals[4]);
    req.operation.arg2 = ntohl(vals[5]);

    return 6 * sizeof(int);
}