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

Robot ClientStub::order_robot(const RobotOrder& request) {
    Robot response(0, 0, 0, 0, 0);

    if (!socket.send(request)) {
        std::cerr << "[Client " << request.customer_id
                  << "] Failed to send robot order." << std::endl;
        return response;
    }

    if (!socket.receive(response)) {
        std::cerr << "[Client " << request.customer_id
                  << "] Failed to receive robot." << std::endl;
    }

    return response;
}

CustomerRecord ClientStub::read_record(const RobotOrder& request) {
    CustomerRecord record{-1, -1};

    if (!socket.send(request)) {
        std::cerr << "[Client " << request.customer_id
                  << "] Failed to send read-record request." << std::endl;
        return record;
    }

    if (!socket.receive(record)) {
        std::cerr << "[Client " << request.customer_id
                  << "] Failed to receive customer record." << std::endl;
    }

    return record;
}

CustomerRecord ClientStub::scan_records(const RobotOrder& request) {
    CustomerRecord record{-1, -1};

    if (!socket.send(request)) {
        std::cerr << "[Client " << request.customer_id
                  << "] Failed to send scan request." << std::endl;
        return record;
    }

    if (!socket.receive(record)) {
        std::cerr << "[Client " << request.customer_id
                  << "] Failed to receive scan record." << std::endl;
    }

    return record;
}

bool ClientStub::ReadRecords(int customer_id,
                             int orders,
                             LatencyRecorder& recorder) {
    for (int j = 0; j < orders; j++) {

        RobotOrder req(customer_id, -1, 2);

        auto start = std::chrono::high_resolution_clock::now();

        CustomerRecord rec = read_record(req);

        auto end = std::chrono::high_resolution_clock::now();

        auto latency =
            std::chrono::duration_cast<std::chrono::microseconds>(end - start)
                .count();

        {
            std::lock_guard<std::mutex> lock(recorder.mutex);
            recorder.latencies.push_back(latency);
        }

        if (rec.customer_id != -1) {
            std::cout << rec.customer_id << "\t"
                      << rec.last_order << std::endl;
        }
    }

    return true;
}

bool ClientStub::ScanRecords(int max_customer_id,
                             LatencyRecorder& recorder) {
    for (int cid = 0; cid <= max_customer_id; cid++) {
        RobotOrder req(cid, -1, 2);
        auto start = std::chrono::high_resolution_clock::now();
        CustomerRecord rec = read_record(req);
        auto end = std::chrono::high_resolution_clock::now();
        auto latency = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
        {
            std::lock_guard<std::mutex> lock(recorder.mutex);
            recorder.latencies.push_back(latency);
        }
        if (rec.customer_id != -1) {
            std::cout << rec.customer_id << "\t" << rec.last_order << std::endl;
        }
    }

    return true;
}