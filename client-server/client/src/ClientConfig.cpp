#include "../include/ClientConfig.h"

#include <cerrno>
#include <climits>
#include <cstdlib>
#include <iostream>

const std::string ClientConfig::DEFAULT_SERVER_IP = "127.0.0.1";

ClientConfig::ClientConfig()
	: server_ip(DEFAULT_SERVER_IP),
	  server_port(DEFAULT_SERVER_PORT),
	  customers(1),
	  orders(1),
	  robot_type(REGULAR) {}

ClientConfig::ClientConfig(std::string server_ip, int server_port, int customers, int orders, RobotType robot_type)
	: server_ip(std::move(server_ip)),
	  server_port(server_port),
	  customers(customers),
	  orders(orders),
	  robot_type(robot_type) {}

static bool parse_int(const char *value, int &out) {
	if (!value || *value == '\0') {
		return false;
	}

	char *end = nullptr;
	errno = 0;
	long parsed = std::strtol(value, &end, 10);
	if (errno != 0 || end == value || *end != '\0' || parsed > INT_MAX || parsed < INT_MIN) {
		return false;
	}

	out = static_cast<int>(parsed);
	return true;
}

ClientConfig parse_client_config(int argc, char *argv[]) {
	if (argc != 5) {
		std::cerr << "Usage: " << argv[0] << " <server_ip> <server_port> <num_customers> <num_orders>" << std::endl;
		exit(1);
	}

	int port = 0;
	int num_customers = 0;
	int num_orders = 0;

	if (!parse_int(argv[2], port) || port <= 0 || port > 65535) {
		std::cerr << "Invalid port: " << argv[2] << std::endl;
		exit(1);
	}
	if (!parse_int(argv[3], num_customers) || num_customers <= 0) {
		std::cerr << "Invalid number of customers: " << argv[3] << std::endl;
		exit(1);
	}
	if (!parse_int(argv[4], num_orders) || num_orders <= 0) {
		std::cerr << "Invalid number of orders: " << argv[4] << std::endl;
		exit(1);
	}

	return ClientConfig(argv[1], port, num_customers, num_orders, REGULAR);
}
