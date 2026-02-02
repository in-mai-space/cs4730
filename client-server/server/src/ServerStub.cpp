#include "../include/ServerStub.h"
#include "../../common/include/RobotOrder.h"
#include "../../common/include/Robot.h"
#include <iostream>
#include <unistd.h>

void ServerStub::init(ServerSocket* socket) {
    this->socket = socket;
}

void ServerStub::handle_client(int client_fd) {
    std::cout << "Handling client connection: " << client_fd << std::endl;
    
    while (true) {
        RobotOrder order(0, 0, 0);
        
        if (!socket->receive(order, client_fd)) {
            std::cout << "Client " << client_fd << " disconnected" << std::endl;
            break;
        }
        
        std::cout << "Received order - Customer: " << order.customer_id 
                  << ", Order: " << order.order_number 
                  << ", Type: " << order.robot_type << std::endl;
        
        Robot response = process_order(order);
        
        if (!socket->send(response, client_fd)) {
            std::cout << "Failed to send response to client " << client_fd << std::endl;
            break;
        }
        
        std::cout << "Sent robot response to client " << client_fd << std::endl;
    }
    
    close(client_fd);
}

Robot ServerStub::process_order(const RobotOrder& order) {
    Robot robot(order.customer_id, order.order_number, order.robot_type, 1, 1);
    return robot;
}
