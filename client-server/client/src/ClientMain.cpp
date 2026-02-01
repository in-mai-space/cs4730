#include <iostream> 
#include <string>
#include <thread>
#include <vector>

#include "../include/ClientStub.h"
#include "../include/ClientConfig.h"

int startClient(const ClientConfig *config);

int main(int argc, char *argv[]) {
	ClientConfig cfg = parseClientConfig(argc, argv);

	int result = startClient(&cfg);
	if (result != 0) {
		std::cerr << "Client failed to start." << std::endl;
		return result;
	}

	return 0;
}

int startClient(const ClientConfig *cfg) {
	std::vector<std::thread> customer_threads;
	std::vector<std::shared_ptr<ClientStub>> client_stubs;

	int num_customers = cfg->customers;

	for (int i = 0; i < num_customers; i++) {
		// initialize a client stub for each customer
		std::shared_ptr<ClientStub> stub = std::make_shared<ClientStub>();
		client_stubs.push_back(stub);
		stub->Init(cfg->server_ip, cfg->server_port);

		// create a thread for each customer to place orders
		RobotOrder order(i, cfg->orders, cfg->robot_type);
		std::thread t(&ClientStub::Order, stub, order, i);
    	customer_threads.push_back(std::move(t));
	}

	for (auto& thread : customer_threads) {
		if (thread.joinable()) {
			thread.join();
		}
	}

	return 0;
}