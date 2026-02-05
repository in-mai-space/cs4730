#include "../include/ClientStub.h"

#include <chrono>
#include <iostream>

#include "../../common/include/RobotOrder.h"

void ClientStub::init(const std::string& ip, int port) {
    if (!socket.connect(ip, port)) {
        throw std::runtime_error("Failed to connect to server " + ip + ":" + std::to_string(port));
    }
}

bool ClientStub::order(const RobotOrder& order_template, int customer_id, LatencyRecorder& recorder) {
    bool all_success = true;
    for (int i = 1; i <= order_template.order_number; i++) {
        RobotOrder order(customer_id, i, order_template.robot_type);

        auto start_time = std::chrono::high_resolution_clock::now();

        if (!socket.send(order)) {
            std::cerr << "Failed to send order for customer " << customer_id
                      << ", order " << i << std::endl;
            all_success = false;
            continue;
        }

        Robot response(0, 0, 0, 0, 0);

        if (!socket.receive(response)) {
            std::cerr << "Failed to receive response for customer "
                      << customer_id << ", order " << i << std::endl;
            all_success = false;
            continue;
        }

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