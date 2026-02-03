#include <iostream>
#include <thread>
#include <vector>
#include <unistd.h>
#include <queue>
#include <condition_variable>
#include <mutex>
#include <future>

#include "../include/ServerConfig.h"
#include "../include/ServerSocket.h"
#include "../include/ServerStub.h"

void start_server(const ServerConfig& config);
void handle_client_thread(ServerStub* stub, int client_fd, int engineer_id, std::shared_ptr<std::queue<std::promise<Robot>>>& jobQueue, std::mutex &mtx, std::condition_variable &cv);
void initialize_engineer_threads(int id, ServerSocket& server_socket, ServerStub& server_stub, std::vector<std::thread>& engineer_threads, std::shared_ptr<std::queue<std::promise<Robot>>>& jobQueue, std::mutex &mtx, std::condition_variable &cv);
void initialize_expert_engineer_threads(int start_id, std::vector<std::thread>& expert_engineer_threads, ServerStub& server_stub, int num_expert_engineers, std::mutex &mtx,
                             std::shared_ptr<std::queue<std::promise<Robot>>>& jobQueue, std::condition_variable &cv, bool* shutdown_flag);

int main(int argc, char* argv[]) {
    ServerConfig cfg = parse_server_config(argc, argv);
    start_server(cfg);
    return 0;
}

void start_server(const ServerConfig& config) {
    std::cout << "Starting server on port " << config.port << " with "
              << config.expert_engineers << " expert engineers." << std::endl;

    ServerSocket server_socket;
    if (!server_socket.listen(config.port)) {
        std::cerr << "Failed to start server" << std::endl;
        return;
    }

    ServerStub server_stub;
    server_stub.init(&server_socket);
    std::vector<std::thread> engineer_threads;
    std::vector<std::thread> expert_engineer_threads;
    std::mutex mtx;
    std::condition_variable cv;
    std::shared_ptr<std::queue<std::promise<Robot>>> jobQueue = std::make_shared<std::queue<std::promise<Robot>>>();
    int id = 0;

    std::cout << "Server is ready to accept connections..." << std::endl;

    initialize_engineer_threads(id, server_socket, server_stub, engineer_threads, jobQueue, mtx, cv);
    initialize_expert_engineer_threads(id, expert_engineer_threads, server_stub, config.expert_engineers, mtx, jobQueue, cv, nullptr);
}

void handle_client_thread(ServerStub* stub, int client_fd, int engineer_id, std::shared_ptr<std::queue<std::promise<Robot>>>& jobQueue, std::mutex &mtx, std::condition_variable &cv) {
    stub->handle_client(client_fd, engineer_id, jobQueue, mtx, cv);
}

// accept new connections and waits for new connections from client in a loop
void initialize_engineer_threads(int id, ServerSocket& server_socket, ServerStub& server_stub, std::vector<std::thread>& engineer_threads, std::shared_ptr<std::queue<std::promise<Robot>>>& jobQueue, std::mutex &mtx, std::condition_variable &cv) {
    int engineer_id = id; // starting engineer ID

    while (true) {
        int client_fd = server_socket.accept();
        if (client_fd < 0) {
            std::cerr << "Failed to accept client" << std::endl;
            continue;
        }

        std::cout << "New client connected: " << client_fd << std::endl;

        std::thread engineer_thread(handle_client_thread, &server_stub, client_fd, engineer_id++, std::ref(jobQueue), std::ref(mtx), std::ref(cv));
        engineer_thread.detach();
        engineer_threads.push_back(std::move(engineer_thread));
    }
}

void expert_engineers_wait_and_execute_job(int id, std::condition_variable &cv,
                             std::mutex &mtx,
                             std::shared_ptr<std::queue<std::promise<Robot>>>& jobQueue, ServerStub& server_stub, bool* shutdown_flag) {
    while (!*shutdown_flag) {
        std::unique_lock<std::mutex> lock(mtx);

        while (jobQueue->empty() && !*shutdown_flag) {
            cv.wait(lock);
        }
        
        if (*shutdown_flag) {
            break;
        }
        
        auto robot_order = std::move(jobQueue->front());
        jobQueue->pop();

        lock.unlock();
        server_stub.attach_special_module(std::move(robot_order), id);
    }
}

void initialize_expert_engineer_threads(int start_id, std::vector<std::thread>& expert_engineer_threads, ServerStub& server_stub, int num_expert_engineers, std::mutex &mtx,
                             std::shared_ptr<std::queue<std::promise<Robot>>>& jobQueue, std::condition_variable &cv, bool* shutdown_flag) {
    for (int i = 0; i < num_expert_engineers; ++i) {
        std::thread expert_thread(expert_engineers_wait_and_execute_job, start_id + i, std::ref(cv), std::ref(mtx), std::ref(jobQueue), std::ref(server_stub), shutdown_flag);
        expert_thread.detach();
        expert_engineer_threads.push_back(std::move(expert_thread));
    }
}