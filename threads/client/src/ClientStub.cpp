#include "../include/ClientStub.h"

#include <chrono>
#include <iostream>

#include "../../common/include/RobotOrder.h"

void ClientStub::init(const std::string& ip, int port) {
    if (!socket.connect(ip, port)) {
        throw std::runtime_error("Client terminates gracefully");
    }
}

bool ClientStub::order(const RobotOrder& order_template, int customer_id,
                       LatencyRecorder& recorder) {
    bool all_success = true;
    for (int i = 1; i <= order_template.order_number; i++) {
        RobotOrder order(customer_id, i, order_template.robot_type);

        std::cout << "[Client " << customer_id << "] Sending order " << i
                  << " (robot_type=" << order_template.robot_type
                  << ") to server..." << std::endl;
        auto start_time = std::chrono::high_resolution_clock::now();

        if (!socket.send(order)) {
            std::cerr << "[Client " << customer_id << "] Failed to send order "
                      << i << std::endl;
            all_success = false;
            continue;
        }
        std::cout << "[Client " << customer_id << "] Order " << i
                  << " sent. Waiting for robot..." << std::endl;

        Robot response(0, 0, 0, 0, 0);

        if (!socket.receive(response)) {
            std::cerr << "[Client " << customer_id
                      << "] Failed to receive response for order " << i
                      << std::endl;
            all_success = false;
            continue;
        }
        std::cout << "[Client " << customer_id << "] Received robot for order "
                  << i << ": engineer_id=" << response.engineer_id
                  << ", expert_id=" << response.expert_id << std::endl;

        // accumulate time taken to process an order for calculation at the end
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