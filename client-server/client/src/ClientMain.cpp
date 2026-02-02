#include <iostream> 
#include <string>
#include <thread>
#include <vector>
#include <chrono>
#include <mutex>
#include <atomic>
#include <algorithm>

#include "../include/ClientStub.h"
#include "../include/ClientConfig.h"

void start_client(const ClientConfig *config);

int main(int argc, char *argv[]) {
	ClientConfig cfg = parse_client_config(argc, argv);
	start_client(&cfg);
	return 0;
}

void start_client(const ClientConfig *cfg) {
	std::vector<std::thread> customer_threads;
	std::vector<std::shared_ptr<ClientStub>> client_stubs;
	std::vector<long long> latencies;
	std::mutex latency_mutex;

	int num_customers = cfg->customers;

	auto start_time = std::chrono::high_resolution_clock::now();

	for (int i = 0; i < num_customers; i++) {
		std::shared_ptr<ClientStub> stub = std::make_shared<ClientStub>();
		client_stubs.push_back(stub);
		stub->init(cfg->server_ip, cfg->server_port);

		RobotOrder order_template(i, cfg->orders, cfg->robot_type);
		std::thread t(&ClientStub::order, stub, order_template, i, std::ref(latencies), std::ref(latency_mutex));
    	customer_threads.push_back(std::move(t));
	}

	for (auto& thread : customer_threads) {
		if (thread.joinable()) {
			thread.join();
		}
	}

	auto end_time = std::chrono::high_resolution_clock::now();
	
	if (!latencies.empty()) {
		long long sum = 0;
		long long min_latency = latencies[0];
		long long max_latency = latencies[0];
		
		for (long long latency : latencies) {
			sum += latency;
			min_latency = std::min(min_latency, latency);
			max_latency = std::max(max_latency, latency);
		}
		
		double avg_latency = static_cast<double>(sum) / latencies.size();
		
		auto total_time_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();
		double total_time_seconds = static_cast<double>(total_time_ms) / 1000.0;
		double throughput = static_cast<double>(latencies.size()) / total_time_seconds;
		
		std::cout << avg_latency << "\t" << min_latency << "\t" << max_latency << "\t" << throughput << std::endl;
	}
}