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

void ServerStub::attach_special_module(std::promise<Robot>&& promise, int expert_id) {
    std::this_thread::sleep_for(std::chrono::seconds(1));
    Robot modified_robot = promise.get_future().get();
    modified_robot.expert_id = expert_id;
    promise.set_value(modified_robot);
}

void ServerStub::handle_client(int client_fd, int engineer_id, ExpertRequestQueue& expertQueue) {
    RobotOrder order(0, 0, 0);

    while (true) {
        if (!receive_order(order, client_fd)) {
            break;
        }

        Robot response = process_order(order, engineer_id);

        if (is_special_robot(response.robot_type)) {
            std::promise<Robot> completion_promise;
            std::future<Robot> completion_future = completion_promise.get_future();
            {
                std::lock_guard<std::mutex> lock(expertQueue.mtx);
                expertQueue.jobQueue.push(std::move(completion_promise));
            }
            expertQueue.cv.notify_one();
            response = completion_future.get();
        }

        if (!ship_robot(response, client_fd)) {
            std::cout << "Failed to send response to client " << client_fd
                      << std::endl;
            break;
        }
    }

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