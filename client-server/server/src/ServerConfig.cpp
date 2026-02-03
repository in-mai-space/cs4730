#include "../include/ServerConfig.h"

#include <cerrno>
#include <climits>
#include <cstdlib>
#include <iostream>

ServerConfig::ServerConfig()
	: port(DEFAULT_SERVER_PORT), expert_engineers(1) {}

ServerConfig::ServerConfig(int port, int *expert_engineers)
	: port(port), expert_engineers(*expert_engineers) {}

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

ServerConfig parse_server_config(int argc, char *argv[]) {
	if (argc < 2 || argc > 3) {
		std::cerr << "Usage: " << argv[0] << " <server_port> [num_expert_engineers]" << std::endl;
		exit(1);
	}

	int port = 0;
	int num_expert_engineers = 0;

	if (!parse_int(argv[1], port) || port <= 0 || port > 65535) {
		std::cerr << "Invalid port: " << argv[1] << std::endl;
		exit(1);
	}
	
	if (argc == 3) {
		if (!parse_int(argv[2], num_expert_engineers) || num_expert_engineers < 0) {
			std::cerr << "Invalid number of expert engineers: " << argv[2] << std::endl;
			exit(1);
		}
	}

	return ServerConfig(port, &num_expert_engineers);
}
