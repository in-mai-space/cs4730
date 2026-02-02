#include <iostream>
#include <thread>
#include <vector>
#include "../include/ServerConfig.h"
#include "../include/ServerSocket.h"
#include "../include/ServerStub.h"

void start_server(const ServerConfig& config);
void handle_client_thread(ServerStub* stub, int client_fd);

int main(int argc, char *argv[]) {
	ServerConfig cfg = parse_server_config(argc, argv);
	start_server(cfg);
	return 0;
}

void start_server(const ServerConfig& config) {
	std::cout << "Starting server on port " << config.port 
			  << " with " << config.expert_engineers << " expert engineers." << std::endl;

	ServerSocket server_socket;
	if (!server_socket.listen(config.port)) {
		std::cerr << "Failed to start server" << std::endl;
		return;
	}

	ServerStub server_stub;
	server_stub.init(&server_socket);

	std::vector<std::thread> client_threads;

	std::cout << "Server is ready to accept connections..." << std::endl;

	while (true) {
		int client_fd = server_socket.accept();
		if (client_fd < 0) {
			std::cerr << "Failed to accept client" << std::endl;
			continue;
		}

		std::cout << "New client connected: " << client_fd << std::endl;

		std::thread client_thread(handle_client_thread, &server_stub, client_fd);
		client_thread.detach();
	}
}

void handle_client_thread(ServerStub* stub, int client_fd) {
	stub->handle_client(client_fd);
}