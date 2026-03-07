#ifndef CLIENTSOCKET_H
#define CLIENTSOCKET_H

#include <cstddef>
#include <string>

#include "../../common/include/CustomerRecords.h"
#include "../../common/include/Robot.h"
#include "../../common/include/RobotOrder.h"

class ClientSocket {
   public:
    /**
     * Constructor
     */
    ClientSocket();

    /**
     * Connects to the server at the specified IP and port.
     * Automatically sends identification as a customer (type 0).
     * @param ip The server IP address.
     * @param port The server port number.
     */
    bool connect(const std::string& ip, int port);

    /**
     * Sends an identification message to the server.
     */
    bool identify_as_customer();

    /**
     * Sends a RobotOrder to the server.
     * @param order The RobotOrder to send.
     */
    bool send(const RobotOrder& order);

    /**
     * Receives a Robot from the server.
     * @param order The Robot object to populate with received data.
     */
    bool receive(Robot& order);

    /**
     * Receives a CustomerRecord from the server.
     * @param record The CustomerRecord object to populate with received data.
     */
    bool receive(CustomerRecord& record);

   private:
    int marshall(const RobotOrder& order, char* buffer, int buffer_size);
    int unmarshall(const char* buffer, int buffer_size, Robot& order);
    int unmarshall(const char* buffer, int buffer_size, CustomerRecord& record);
    bool send_all(const char* data, size_t len);
    bool receive_all(char* data, size_t len);
    int sock_fd;
};

#endif
