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

void ServerStub::handle_client(RobotOrder& order, int client_fd, int engineer_id, std::shared_ptr<std::queue<std::promise<Robot>>>& jobQueue, std::mutex &mtx, std::condition_variable &cv) {
    while (true) {
        if (!socket->receive(order, client_fd)) {
            break;
        }

        Robot response = process_order(order, engineer_id);

        if (is_special_robot(response.robot_type)) {
            std::promise<Robot> completion_promise;
            std::future<Robot> completion_future = completion_promise.get_future();
            {
                std::lock_guard<std::mutex> lock(mtx);
                jobQueue->push(std::move(completion_promise));
            }
            cv.notify_one();
            response = completion_future.get();
        }

        if (!socket->send(response, client_fd)) {
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