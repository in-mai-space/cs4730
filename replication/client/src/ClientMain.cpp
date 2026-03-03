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

void initialize_customer_threads(
    ClientConfig& cfg, std::vector<std::thread>& customer_threads,
    std::vector<std::shared_ptr<ClientStub>>& client_stubs,
    LatencyRecorder& recorder) {
    for (int i = 0; i < cfg.customers; i++) {
        std::shared_ptr<ClientStub> stub = std::make_shared<ClientStub>();
        client_stubs.push_back(stub);
        stub->init(cfg.server_ip, cfg.server_port);

        if (cfg.request_type == 1) {
            // Robot order: stub loops orders times, receives Robot per order.
            RobotOrder tmpl(i, cfg.orders, 1);
            std::thread t(&ClientStub::Order, stub, tmpl, i,
                          std::ref(recorder));
            customer_threads.push_back(std::move(t));

        } else if (cfg.request_type == 2) {
            // Record read: send cfg.orders read requests for own customer_id.
            std::thread t([stub, i, &cfg, &recorder]() {
                for (int j = 0; j < cfg.orders; j++) {
                    RobotOrder req(i, -1, 2);
                    auto t0 = std::chrono::high_resolution_clock::now();
                    CustomerRecord rec = stub->ReadRecord(req);
                    auto t1 = std::chrono::high_resolution_clock::now();
                    auto latency =
                        std::chrono::duration_cast<std::chrono::microseconds>(
                            t1 - t0)
                            .count();
                    {
                        std::lock_guard<std::mutex> lock(recorder.mutex);
                        recorder.latencies.push_back(latency);
                    }
                    if (rec.customer_id != -1) {
                        std::cout << rec.customer_id << "\t" << rec.last_order
                                  << std::endl;
                    }
                }
            });
            customer_threads.push_back(std::move(t));

        } else if (cfg.request_type == 3) {
            // Type 3: scan customer IDs 0..orders and print all valid records.
            std::thread t([stub, &cfg, &recorder]() {
                for (int cid = 0; cid <= cfg.orders; cid++) {
                    RobotOrder req(cid, -1, 2);
                    auto t0 = std::chrono::high_resolution_clock::now();
                    CustomerRecord rec = stub->ReadRecord(req);
                    auto t1 = std::chrono::high_resolution_clock::now();
                    auto latency =
                        std::chrono::duration_cast<std::chrono::microseconds>(
                            t1 - t0)
                            .count();
                    {
                        std::lock_guard<std::mutex> lock(recorder.mutex);
                        recorder.latencies.push_back(latency);
                    }
                    if (rec.customer_id != -1) {
                        std::cout << rec.customer_id << "\t" << rec.last_order
                                  << std::endl;
                    }
                }
            });
            customer_threads.push_back(std::move(t));
        }
    }

    for (auto& thread : customer_threads) {
        if (thread.joinable()) {
            thread.join();
        }
    }
}
