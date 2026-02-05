#include "../include/ServerStub.h"

#include <unistd.h>
#include <queue>
#include <thread>
#include <chrono>

#include <iostream>

#include "../../common/include/Robot.h"
#include "../../common/include/RobotOrder.h"

void ServerStub::init(ServerSocket* socket) { this->socket = socket; }

Robot ServerStub::process_order(const RobotOrder& order, int engineer_id) {
    Robot robot(order.customer_id, order.order_number, order.robot_type, engineer_id, -1); // -1 since expert_id is not assigned yet
    return robot;
}

void ServerStub::attach_special_module(ExpertRequest req, int expert_id) {
    std::cout << "[Expert Engineer " << expert_id << "] Received robot, adding special module..." << std::endl;
    std::this_thread::sleep_for(std::chrono::microseconds(100));
    req.robot.expert_id = expert_id;
    std::cout << "[Expert Engineer " << expert_id << "] Special module added, returning robot." << std::endl;
    req.promise.set_value(req.robot);
}

void ServerStub::handle_client(int client_fd, int engineer_id, ExpertRequestQueue& expertQueue) {
    RobotOrder order(0, 0, 0);

    while (true) {
        std::cout << "[Engineer " << engineer_id << "] Waiting for order from client " << client_fd << std::endl;
        if (!receive_order(order, client_fd)) {
            std::cout << "[Engineer " << engineer_id << "] Client " << client_fd << " disconnected or error receiving order." << std::endl;
            break;
        }
        std::cout << "[Engineer " << engineer_id << "] Received order: customer_id=" << order.customer_id << ", order_number=" << order.order_number << ", robot_type=" << order.robot_type << std::endl;

        Robot response = process_order(order, engineer_id);
        std::cout << "[Engineer " << engineer_id << "] Processed order, robot info: customer_id=" << response.customer_id << ", order_number=" << response.order_number << ", robot_type=" << response.robot_type << ", engineer_id=" << response.engineer_id << std::endl;

        if (is_special_robot(response.robot_type)) {
            std::cout << "[Engineer " << engineer_id << "] Special robot requested, sending to expert queue..." << std::endl;
            ExpertRequest req{response, std::promise<Robot>()};
            std::future<Robot> completion_future = req.promise.get_future();
            {
                std::lock_guard<std::mutex> lock(expertQueue.mtx);
                expertQueue.jobQueue.push(std::move(req));
            }
            expertQueue.cv.notify_one();
            response = completion_future.get();
            std::cout << "[Engineer " << engineer_id << "] Received robot with expert module, expert_id=" << response.expert_id << std::endl;
        }

        std::cout << "[Engineer " << engineer_id << "] Shipping robot to client " << client_fd << std::endl;
        if (!ship_robot(response, client_fd)) {
            std::cout << "[Engineer " << engineer_id << "] Failed to send response to client " << client_fd << std::endl;
            break;
        }
        std::cout << "[Engineer " << engineer_id << "] Robot shipped to client " << client_fd << std::endl;
    }

    std::cout << "[Engineer " << engineer_id << "] Closing connection to client " << client_fd << std::endl;
    close(client_fd);
}

bool ServerStub::is_special_robot(int robot_type) {
    switch (robot_type) {
        case 1:
            return true;
        case 0:
            return false;
        default:
            throw std::invalid_argument("Invalid robot type");
    }
}

bool ServerStub::receive_order(RobotOrder& order, int client_fd) {
    return socket && socket->receive(order, client_fd);
}

bool ServerStub::ship_robot(const Robot& robot, int client_fd) {
    return socket && socket->send(robot, client_fd);
}