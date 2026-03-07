#include "../include/ClientStub.h"

#include <chrono>
#include <iostream>
#include <mutex>

void ClientStub::init(const std::string& ip, int port) {
    if (!socket.connect(ip, port)) {
        throw std::runtime_error("Client terminates gracefully");
    }
}

bool ClientStub::Order(const RobotOrder& order_template, int customer_id,
                       LatencyRecorder& recorder) {
    for (int i = 1; i <= order_template.order_number && running; i++) {
        RobotOrder request(customer_id, i, 1);

        std::cout << "[Client " << customer_id << "] Sending robot order " << i
                  << std::endl;

        auto start_time = std::chrono::high_resolution_clock::now();

        if (!socket.send(request)) {
            std::cerr << "[Client " << customer_id
                      << "] Primary server disconnected." << std::endl;
            running = false;
            return false;
        }

        Robot response(0, 0, 0, 0, 0);

        if (!socket.receive(response)) {
            running = false;
            return false;
        }

        std::cout << "[Client " << customer_id
                  << "] Received robot: engineer_id=" << response.engineer_id
                  << ", admin_id=" << response.admin_id << std::endl;

        auto end_time = std::chrono::high_resolution_clock::now();

        long latency = std::chrono::duration_cast<std::chrono::microseconds>(
                           end_time - start_time)
                           .count();

        {
            std::lock_guard<std::mutex> lock(recorder.mutex);
            recorder.latencies.push_back(latency);
        }
    }

    return true;
}

CustomerRecord ClientStub::read_record(const RobotOrder& request) {
    CustomerRecord record{-1, -1};

    if (!socket.send(request)) {
        running = false;
        return record;
    }

    if (!socket.receive(record)) {
        running = false;
        return record;
    }

    return record;
}

bool ClientStub::ReadRecords(int customer_id, int orders,
                             LatencyRecorder& recorder) {
    for (int j = 0; j < orders && running; j++) {
        RobotOrder req(customer_id, -1, 2);

        auto start = std::chrono::high_resolution_clock::now();

        CustomerRecord rec = read_record(req);

        auto end = std::chrono::high_resolution_clock::now();

        long latency =
            std::chrono::duration_cast<std::chrono::microseconds>(end - start)
                .count();

        {
            std::lock_guard<std::mutex> lock(recorder.mutex);
            recorder.latencies.push_back(latency);
        }

        if (rec.customer_id != -1) {
            std::cout << rec.customer_id << "\t" << rec.last_order << std::endl;
        }
    }

    return running;
}

bool ClientStub::ScanRecords(int max_customer_id, LatencyRecorder& recorder) {
    for (int cid = 0; cid <= max_customer_id && running; cid++) {
        RobotOrder req(cid, -1, 2);

        auto start = std::chrono::high_resolution_clock::now();

        CustomerRecord rec = read_record(req);

        auto end = std::chrono::high_resolution_clock::now();

        long latency =
            std::chrono::duration_cast<std::chrono::microseconds>(end - start)
                .count();

        {
            std::lock_guard<std::mutex> lock(recorder.mutex);
            recorder.latencies.push_back(latency);
        }

        if (rec.customer_id != -1) {
            std::cout << rec.customer_id << "\t" << rec.last_order << std::endl;
        }
    }

    return running;
}