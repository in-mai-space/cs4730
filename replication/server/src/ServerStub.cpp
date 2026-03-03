#include "../include/ServerStub.h"

#include <unistd.h>

#include <iostream>

#include "../../common/include/CustomerRecords.h"
#include "../../common/include/Robot.h"
#include "../../common/include/RobotOrder.h"

void ServerStub::init(ServerSocket* socket) {
    this->socket = socket;
    this->customerRecords = CustomerRecords();
}

// ---- Admin thread: updates log + map, fulfills promise ----
void ServerStub::admin_process_requests(int admin_id,
                                        AdminRequestQueue& adminQueue) {
    std::cout << "[Admin " << admin_id << "] Thread started." << std::endl;
    while (true) {
        std::unique_lock<std::mutex> lock(adminQueue.mtx);
        adminQueue.cv.wait(lock, [&] { return !adminQueue.jobQueue.empty(); });
        AdminRequest req = std::move(adminQueue.jobQueue.front());
        adminQueue.jobQueue.pop();
        lock.unlock();

        std::cout << "[Admin " << admin_id
                  << "] Processing customer_id=" << req.robot.customer_id
                  << " order_number=" << req.robot.order_number << std::endl;

        {
            std::lock_guard<std::mutex> wlock(records_mutex);
            smr_log.add_operation(1, req.robot.customer_id,
                                  req.robot.order_number);
            customerRecords.update_record(req.robot.customer_id,
                                          req.robot.order_number);
        }

        req.robot.admin_id = admin_id;
        req.promise.set_value(req.robot);
    }
}

void ServerStub::handle_client_request(int client_fd, int engineer_id,
                                       AdminRequestQueue& adminQueue) {
    RobotOrder request(0, 0, 0);

    while (true) {
        if (!ReceiveRequest(request, client_fd)) {
            std::cout << "[Engineer " << engineer_id << "] Client " << client_fd
                      << " disconnected." << std::endl;
            break;
        }

        std::cout << "[Engineer " << engineer_id
                  << "] Received request: customer_id=" << request.customer_id
                  << ", order_number=" << request.order_number
                  << ", request_type=" << request.request_type << std::endl;

        if (request.request_type == 1) {
            Robot robot(request.customer_id, request.order_number,
                        request.request_type, engineer_id, -1);

            std::promise<Robot> p;
            std::future<Robot> fut = p.get_future();
            {
                std::lock_guard<std::mutex> lock(adminQueue.mtx);
                adminQueue.jobQueue.push(AdminRequest{robot, std::move(p)});
            }
            adminQueue.cv.notify_one();

            robot = fut.get();
            std::cout << "[Engineer " << engineer_id
                      << "] Shipping robot (admin_id=" << robot.admin_id
                      << ")" << std::endl;
            if (!ShipRobot(robot, client_fd)) {
                std::cerr << "[Engineer " << engineer_id
                          << "] Failed to ship robot." << std::endl;
                break;
            }

        } else if (request.request_type == 2) {
            CustomerRecord record;
            {
                std::lock_guard<std::mutex> rlock(records_mutex);
                record = customerRecords.get_record(request.customer_id);
            }
            std::cout << "[Engineer " << engineer_id
                      << "] Returning record: customer_id="
                      << record.customer_id
                      << ", last_order=" << record.last_order << std::endl;
            if (!ReturnRecord(record, client_fd)) {
                std::cerr << "[Engineer " << engineer_id
                          << "] Failed to return record." << std::endl;
                break;
            }

        } else {
            std::cerr << "[Engineer " << engineer_id
                      << "] Unknown request_type=" << request.request_type
                      << std::endl;
            break;
        }
    }

    std::cout << "[Engineer " << engineer_id << "] Closing connection "
              << client_fd << std::endl;
    close(client_fd);
}

bool ServerStub::ReceiveRequest(RobotOrder& request, int client_fd) {
    return socket && socket->receive(request, client_fd);
}

bool ServerStub::ShipRobot(const Robot& robot, int client_fd) {
    return socket && socket->send(robot, client_fd);
}

bool ServerStub::ReturnRecord(const CustomerRecord& record, int client_fd) {
    return socket && socket->send(record, client_fd);
}