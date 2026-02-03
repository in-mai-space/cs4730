#include "../include/ClientStub.h"

#include <chrono>
#include <iostream>

#include "../../common/include/RobotOrder.h"

void ClientStub::init(std::string ip, int port) {
    if (!socket.connect(ip, port)) {
        std::cerr << "Failed to connect to server " << ip << ":" << port
                  << std::endl;
        exit(1);
    }
}

void ClientStub::order(RobotOrder order_template, int customer_id,
                       std::vector<long long>& latencies,
                       std::mutex& latency_mutex) {
    for (int i = 1; i <= order_template.order_number; i++) {
        RobotOrder order(customer_id, i, order_template.robot_type);

        // start timer before request
        auto start_time = std::chrono::high_resolution_clock::now();

        // check if send operation succeeds
        if (!socket.send(order)) {
            std::cerr << "Failed to send order for customer " << customer_id
                      << ", order " << i << std::endl;
            continue;  // skip this order and continue with next
        }

        Robot response(0, 0, 0, 0, 0);

        // check if receive operation succeeds
        if (!socket.receive(response)) {
            std::cerr << "Failed to receive response for customer "
                      << customer_id << ", order " << i << std::endl;
            continue;  // skip this order and continue with next
        }

        // end timer after response
        auto end_time = std::chrono::high_resolution_clock::now();

        // calculate latency in microseconds
        auto latency = std::chrono::duration_cast<std::chrono::microseconds>(
                           end_time - start_time)
                           .count();

        // lock and store latency
        {
            std::lock_guard<std::mutex> lock(latency_mutex);
            latencies.push_back(latency);
        }
    }
}