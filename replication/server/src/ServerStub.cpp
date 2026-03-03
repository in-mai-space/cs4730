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

void ServerStub::admin_process_requests(int admin_id,
                                        AdminRequestQueue& adminQueue) {
    std::cout << "[Admin " << admin_id << "] Thread started." << std::endl;
    while (true) {
        // wait for the next request from an engineer thread
        std::unique_lock<std::mutex> lock(adminQueue.mtx);
        adminQueue.cv.wait(lock, [&] { return !adminQueue.jobQueue.empty(); });
        AdminRequest req = std::move(adminQueue.jobQueue.front());
        adminQueue.jobQueue.pop();
        lock.unlock();

        std::cout << "[Admin " << admin_id
                  << "] Processing customer_id=" << req.robot.customer_id
                  << " order_number=" << req.robot.order_number << std::endl;

        // update the customer record and state machine log
        {
            std::lock_guard<std::mutex> wlock(records_mutex);
            smr_log.add_operation(1, req.robot.customer_id,
                                  req.robot.order_number);
            customerRecords.update_record(req.robot.customer_id,
                                          req.robot.order_number);
        }

        // fulfill the promise to unblock the engineer thread
        req.robot.admin_id = admin_id;
        req.promise.set_value(req.robot);
    }
}

bool ServerStub::handle_robot_order(const RobotOrder& request, int engineer_id,
                                    int client_fd,
                                    AdminRequestQueue& adminQueue) {
    Robot robot(request.customer_id, request.order_number,
                request.request_type, engineer_id, -1);

    // send the request to the admin thread and wait for the response
    std::promise<Robot> p;
    std::future<Robot> fut = p.get_future();
    {
        std::lock_guard<std::mutex> lock(adminQueue.mtx);
        adminQueue.jobQueue.push(AdminRequest{robot, std::move(p)});
    }

    // notify the admin thread that a new request is available
    adminQueue.cv.notify_one();

    // wait for the admin thread to process the request and return the updated robot
    robot = fut.get();
    std::cout << "[Engineer " << engineer_id
              << "] Shipping robot (admin_id=" << robot.admin_id << ")"
              << std::endl;
    return ShipRobot(robot, client_fd);
}

bool ServerStub::handle_record_read(const RobotOrder& request, int engineer_id,
                                    int client_fd) {
    CustomerRecord record;
    {
        std::lock_guard<std::mutex> rlock(records_mutex);
        record = customerRecords.get_record(request.customer_id);
    }
    std::cout << "[Engineer " << engineer_id
              << "] Returning record: customer_id=" << record.customer_id
              << ", last_order=" << record.last_order << std::endl;
    return ReturnRecord(record, client_fd);
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

        bool ok = false;
        if (request.request_type == 1) {
            ok = handle_robot_order(request, engineer_id, client_fd,
                                    adminQueue);
        } else if (request.request_type == 2) {
            ok = handle_record_read(request, engineer_id, client_fd);
        } else {
            std::cerr << "[Engineer " << engineer_id
                      << "] Unknown request_type=" << request.request_type
                      << std::endl;
            break;
        }

        if (!ok) {
            std::cerr << "[Engineer " << engineer_id
                      << "] Failed to send response, closing connection."
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