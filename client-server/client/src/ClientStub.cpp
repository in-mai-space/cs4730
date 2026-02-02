#include "../include/ClientStub.h"
#include "../../common/include/RobotOrder.h"
#include <chrono>

void ClientStub::init(std::string ip, int port) {
    socket.connect(ip, port);
}

void ClientStub::order(RobotOrder order_template, int customer_id, std::vector<long long>& latencies, std::mutex& latency_mutex) {
    for (int i = 1; i <= order_template.order_number; i++) {
        RobotOrder order(customer_id, i, order_template.robot_type);

        auto start_time = std::chrono::high_resolution_clock::now();
        
        socket.send(order);
        Robot response(0, 0, 0, 0, 0);
        socket.receive(response);
        
        auto end_time = std::chrono::high_resolution_clock::now();
        
        auto latency = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time).count();
        
        {
            std::lock_guard<std::mutex> lock(latency_mutex);
            latencies.push_back(latency);
        }
    }
}