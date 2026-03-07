#include <algorithm>
#include <atomic>
#include <chrono>
#include <iostream>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

#include "../../common/include/CustomerRecords.h"
#include "../../common/include/RobotOrder.h"
#include "../include/ClientConfig.h"
#include "../include/ClientLogger.h"
#include "../include/ClientStub.h"

void start_client(ClientConfig& config);
void initialize_customer_threads(
    ClientConfig& cfg, std::vector<std::thread>& customer_threads,
    std::vector<std::shared_ptr<ClientStub>>& client_stubs,
    LatencyRecorder& recorder);

int main(int argc, char* argv[]) {
    ClientConfig cfg = parse_and_validate_client_config(argc, argv);
    start_client(cfg);
    return 0;
}

void start_client(ClientConfig& cfg) {
    std::vector<std::thread> customer_threads;
    std::vector<std::shared_ptr<ClientStub>> client_stubs;
    LatencyRecorder recorder;

    auto start_time = std::chrono::high_resolution_clock::now();
    initialize_customer_threads(cfg, customer_threads, client_stubs, recorder);
    auto end_time = std::chrono::high_resolution_clock::now();

    auto logger = ClientLogger(recorder.latencies);
    logger.log_performance_statistics(start_time, end_time);
}

std::thread start_robot_order_thread(std::shared_ptr<ClientStub> stub,
                                     int customer_id,
                                     ClientConfig& cfg,
                                     LatencyRecorder& recorder) {
    RobotOrder tmpl(customer_id, cfg.orders, 1);
    return std::thread(&ClientStub::Order, stub, tmpl, customer_id,
                       std::ref(recorder));
}

std::thread start_record_read_thread(std::shared_ptr<ClientStub> stub,
                                     int customer_id,
                                     ClientConfig& cfg,
                                     LatencyRecorder& recorder) {

    return std::thread(&ClientStub::ReadRecords,
                       stub,
                       customer_id,
                       cfg.orders,
                       std::ref(recorder));
}

std::thread start_scan_thread(std::shared_ptr<ClientStub> stub,
                              ClientConfig& cfg,
                              LatencyRecorder& recorder) {

    return std::thread(&ClientStub::ScanRecords,
                       stub,
                       cfg.orders,
                       std::ref(recorder));
}

void record_latency(LatencyRecorder& recorder,
                    std::chrono::high_resolution_clock::time_point start,
                    std::chrono::high_resolution_clock::time_point end) {
    auto latency = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
    std::lock_guard<std::mutex> lock(recorder.mutex);
    recorder.latencies.push_back(latency);
}

void initialize_customer_threads(
    ClientConfig& cfg,
    std::vector<std::thread>& customer_threads,
    std::vector<std::shared_ptr<ClientStub>>& client_stubs,
    LatencyRecorder& recorder) {

    for (int i = 0; i < cfg.customers; i++) {
        auto stub = std::make_shared<ClientStub>();
        stub->init(cfg.server_ip, cfg.server_port);
        client_stubs.push_back(stub);

        switch (cfg.request_type) {
            case 1:
                customer_threads.push_back(
                    start_robot_order_thread(stub, i, cfg, recorder));
                break;

            case 2:
                customer_threads.push_back(
                    start_record_read_thread(stub, i, cfg, recorder));
                break;

            case 3:
                customer_threads.push_back(
                    start_scan_thread(stub, cfg, recorder)); // only start one thread for scanning
                break;
        }
    }

    for (auto& thread : customer_threads) {
        if (thread.joinable()) {
            thread.join();
        }
    }
}