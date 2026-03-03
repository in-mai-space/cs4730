#include "../include/ClientSocket.h"

#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cstring>

#include "../../common/include/CustomerRecords.h"

ClientSocket::ClientSocket() : sock_fd(-1) {}

bool ClientSocket::connect(const std::string& ip, int port) {
    sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_fd < 0) return false;

    sockaddr_in serv_addr{};
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);

    if (inet_pton(AF_INET, ip.c_str(), &serv_addr.sin_addr) <= 0) {
        close(sock_fd);
        sock_fd = -1;
        return false;
    }

    if (::connect(sock_fd, (sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        close(sock_fd);
        sock_fd = -1;
        return false;
    }

    return true;
}

bool ClientSocket::send(const RobotOrder& order) {
    char buffer[sizeof(RobotOrder)];
    int len = marshall(order, buffer, sizeof(buffer));
    if (len <= 0) return false;

    return send_all(buffer, len);
}

bool ClientSocket::receive(Robot& order) {
    char buffer[sizeof(Robot)];

    if (!receive_all(buffer, sizeof(buffer))) return false;

    return unmarshall(buffer, sizeof(buffer), order) > 0;
}

bool ClientSocket::send_all(const char* data, size_t len) {
    size_t total = 0;
    while (total < len) {
        ssize_t sent = ::send(sock_fd, data + total, len - total, 0);
        if (sent <= 0) return false;
        total += sent;
    }
    return true;
}

bool ClientSocket::receive_all(char* data, size_t len) {
    size_t total = 0;
    while (total < len) {
        ssize_t recvd = ::recv(sock_fd, data + total, len - total, 0);
        if (recvd <= 0) return false;
        total += recvd;
    }
    return true;
}

int ClientSocket::marshall(const RobotOrder& order, char* buffer,
                           int buffer_size) {
    if (buffer_size < 3 * (int)sizeof(int)) return -1;

    int net_customer_id = htonl(order.customer_id);
    int net_order_number = htonl(order.order_number);
    int net_request_type = htonl(order.request_type);

    std::memcpy(buffer, &net_customer_id, sizeof(int));
    std::memcpy(buffer + sizeof(int), &net_order_number, sizeof(int));
    std::memcpy(buffer + 2 * sizeof(int), &net_request_type, sizeof(int));

    return 3 * sizeof(int);
}

int ClientSocket::unmarshall(const char* buffer, int buffer_size,
                             Robot& order) {
    if (buffer_size < 5 * (int)sizeof(int)) return -1;

    int net_customer_id = 0, net_order_number = 0, net_request_type = 0;
    int net_engineer_id = 0, net_admin_id = 0;

    std::memcpy(&net_customer_id, buffer, sizeof(int));
    std::memcpy(&net_order_number, buffer + sizeof(int), sizeof(int));
    std::memcpy(&net_request_type, buffer + 2 * sizeof(int), sizeof(int));
    std::memcpy(&net_engineer_id, buffer + 3 * sizeof(int), sizeof(int));
    std::memcpy(&net_admin_id, buffer + 4 * sizeof(int), sizeof(int));

    order.customer_id = ntohl(net_customer_id);
    order.order_number = ntohl(net_order_number);
    order.request_type = ntohl(net_request_type);
    order.engineer_id = ntohl(net_engineer_id);
    order.admin_id = ntohl(net_admin_id);

    return 5 * sizeof(int);
}

bool ClientSocket::receive(CustomerRecord& record) {
    char buffer[2 * sizeof(int)];
    if (!receive_all(buffer, sizeof(buffer))) return false;
    return unmarshall(buffer, sizeof(buffer), record) > 0;
}

int ClientSocket::unmarshall(const char* buffer, int buffer_size,
                             CustomerRecord& record) {
    if (buffer_size < 2 * (int)sizeof(int)) return -1;
    int net_customer_id = 0, net_last_order = 0;
    std::memcpy(&net_customer_id, buffer, sizeof(int));
    std::memcpy(&net_last_order, buffer + sizeof(int), sizeof(int));
    record.customer_id = ntohl(net_customer_id);
    record.last_order = ntohl(net_last_order);
    return 2 * sizeof(int);
}
