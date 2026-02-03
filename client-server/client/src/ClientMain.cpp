#include <algorithm>
#include <atomic>
#include <chrono>
#include <iostream>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

#include "../include/ClientConfig.h"
#include "../include/ClientLogger.h"
#include "../include/ClientStub.h"

void start_client(ClientConfig& config);
void initialize_customer_threads(
    ClientConfig& cfg, std::vector<std::thread>& customer_threads,
    std::vector<std::shared_ptr<ClientStub>>& client_stubs,
    std::vector<long long>& latencies, std::mutex& latency_mutex);

int main(int argc, char* argv[]) {
    ClientConfig cfg = parse_and_validate_client_config(argc, argv);
    start_client(cfg);
    return 0;
}

/**
 * Starts the client by initializing customer threads and calculating latencies.
 */
void start_client(ClientConfig& cfg) {
    std::vector<std::thread> customer_threads;
    std::vector<std::shared_ptr<ClientStub>> client_stubs;
    std::vector<long long> latencies;
    std::mutex latency_mutex;

    // record start time for throughput calculation
    auto start_time = std::chrono::high_resolution_clock::now();

    initialize_customer_threads(cfg, customer_threads, client_stubs, latencies,
                                latency_mutex);

    // record end time for throughput calculation
    auto end_time = std::chrono::high_resolution_clock::now();

    auto logger = ClientLogger(latencies);
    logger.log_performance_statistics(start_time, end_time);
}

/**
 * Initializes customer threads to place orders concurrently.
 */
void initialize_customer_threads(
    ClientConfig& cfg, std::vector<std::thread>& customer_threads,
    std::vector<std::shared_ptr<ClientStub>>& client_stubs,
    std::vector<long long>& latencies, std::mutex& latency_mutex) {
    // create the customer threads as many as the specified customer number
    for (int i = 0; i < cfg.customers; i++) {
        // each customer should have its own client stub instance
        std::shared_ptr<ClientStub> stub = std::make_shared<ClientStub>();
        client_stubs.push_back(stub);
        // the socket connection should be made once per client stub
        stub->init(cfg.server_ip, cfg.server_port);

        RobotOrder order(i, cfg.orders, cfg.robot_type);
        // each customer thread should have a unique customer id i
        std::thread t(&ClientStub::order, stub, order, i, std::ref(latencies),
                      std::ref(latency_mutex));
        customer_threads.push_back(std::move(t));
    }

    // wait for all customer threads to finish
    for (auto& thread : customer_threads) {
        if (thread.joinable()) {
            thread.join();
        }
    }
}