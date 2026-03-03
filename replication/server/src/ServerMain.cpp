#include <unistd.h>

#include <condition_variable>
#include <future>
#include <iostream>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

#include "../include/ServerConfig.h"
#include "../include/ServerSocket.h"
#include "../include/ServerStub.h"

void start_server(const ServerConfig& config);
void handle_client_thread(ServerStub* stub, int client_fd, int engineer_id,
                          AdminRequestQueue& adminQueue);
void initialize_engineer_threads(int id, ServerSocket& server_socket,
                                 ServerStub& server_stub,
                                 AdminRequestQueue& adminQueue);
void initialize_admin_thread(ServerStub& server_stub,
                             AdminRequestQueue& adminQueue);

int main(int argc, char* argv[]) {
    ServerConfig cfg = parse_server_config(argc, argv);
    start_server(cfg);
    return 0;
}

void start_server(const ServerConfig& config) {
    std::cout << "Starting factory " << config.factory_id << " on port "
              << config.port << " with " << config.peers.size() << " peer(s)."
              << std::endl;

    ServerSocket server_socket;
    if (!server_socket.listen(config.port)) {
        std::cerr << "Failed to start server" << std::endl;
        return;
    }

    ServerStub server_stub;
    server_stub.init(&server_socket, config);

    AdminRequestQueue adminQueue;

    std::cout << "Server is ready to accept connections..." << std::endl;

    // start the single admin thread
    initialize_admin_thread(server_stub, adminQueue);
    // accept connections (blocks forever); engineer IDs start at 1 (admin is 0)
    initialize_engineer_threads(1, server_socket, server_stub, adminQueue);
}

void handle_client_thread(ServerStub* stub, int client_fd, int engineer_id,
                          AdminRequestQueue& adminQueue) {
    stub->handle_client_request(client_fd, engineer_id, adminQueue);
}

// accept new connections in a loop; spawn an engineer thread per client.
void initialize_engineer_threads(int id, ServerSocket& server_socket,
                                 ServerStub& server_stub,
                                 AdminRequestQueue& adminQueue) {
    int engineer_id = id;  // starting ID so it doesn't overlap with admin id 0

    while (true) {
        int client_fd = server_socket.accept();
        if (client_fd < 0) {
            std::cerr << "Failed to accept client" << std::endl;
            continue;
        }
        std::cout << "New client connected: fd=" << client_fd << std::endl;

        std::thread engineer_thread(handle_client_thread, &server_stub,
                                    client_fd, engineer_id++,
                                    std::ref(adminQueue));
        engineer_thread.detach();
    }
}

// start exactly one admin thread
void initialize_admin_thread(ServerStub& server_stub,
                             AdminRequestQueue& adminQueue) {
    std::cout << "[Admin 0] Initializing admin thread." << std::endl;
    std::thread admin_thread(&ServerStub::admin_process_requests, &server_stub,
                             0, std::ref(adminQueue));
    admin_thread.detach();
}