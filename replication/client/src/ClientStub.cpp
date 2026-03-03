#include "../include/ClientStub.h"

#include <chrono>
#include <iostream>

#include "../../common/include/CustomerRecords.h"
#include "../../common/include/RobotOrder.h"

void ClientStub::init(const std::string& ip, int port) {
    if (!socket.connect(ip, port)) {
        throw std::runtime_error("Failed to connect to server " + ip + ":" +
                                 std::to_string(port));
    }
}

bool ClientStub::Order(const RobotOrder& order_template, int customer_id,
                       LatencyRecorder& recorder) {
    bool all_success = true;
    for (int i = 1; i <= order_template.order_number; i++) {
        RobotOrder request(customer_id, i, 1);  // request_type=1: robot order

        std::cout << "[Client " << customer_id << "] Sending robot order " << i
                  << " to server..." << std::endl;
        auto start_time = std::chrono::high_resolution_clock::now();

        if (!socket.send(request)) {
            std::cerr << "[Client " << customer_id << "] Failed to send order "
                      << i << std::endl;
            all_success = false;
            continue;
        }

        Robot response(0, 0, 0, 0, 0);
        if (!socket.receive(response)) {
            std::cerr << "[Client " << customer_id
                      << "] Failed to receive robot for order " << i
                      << std::endl;
            all_success = false;
            continue;
        }
        std::cout << "[Client " << customer_id << "] Received robot for order "
                  << i << ": engineer_id=" << response.engineer_id
                  << ", admin_id=" << response.admin_id << std::endl;

        auto end_time = std::chrono::high_resolution_clock::now();
        auto latency = std::chrono::duration_cast<std::chrono::microseconds>(
                           end_time - start_time)
                           .count();
        {
            std::lock_guard<std::mutex> lock(recorder.mutex);
            recorder.latencies.push_back(latency);
        }
    }
    return all_success;
}

CustomerRecord ClientStub::ReadRecord(const RobotOrder& request) {
    CustomerRecord record{-1, -1};

    if (!socket.send(request)) {
        std::cerr << "[Client " << request.customer_id
                  << "] Failed to send read-record request." << std::endl;
        return record;
    }

    if (!socket.receive(record)) {
        std::cerr << "[Client " << request.customer_id
                  << "] Failed to receive customer record." << std::endl;
        return record;
    }

    return record;
}
