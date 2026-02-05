#ifndef CLIENTSOCKET_H
#define CLIENTSOCKET_H

#include <cstddef>
#include <string>

#include "../../common/include/RobotOrder.h"

class ClientSocket {
   public:
   /**
    * Constructor and Destructor
    */
    ClientSocket();
    ~ClientSocket();

    /**
     * Connects to the server at the specified IP and port.
     * @param ip The server IP address.
     * @param port The server port number.
     */
    bool connect(const std::string& ip, int port);

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

   private:
    /**
     * Marshalls a RobotOrder into a byte buffer.
     * @param order The RobotOrder to marshall.
     * @param buffer The buffer to write the marshalled data into.
     * @param buffer_size The size of the buffer.
     */
    int marshall(const RobotOrder& order, char* buffer, int buffer_size);

    /**
     * Unmarshalls a byte buffer into a Robot.
     * @param buffer The buffer containing the marshalled data.
     * @param buffer_size The size of the buffer.
     * @param order The Robot object to populate with unmarshalled data.
     */
    int unmarshall(const char* buffer, int buffer_size, Robot& order);

    /**
     * Sends all data in the buffer to the server.
     * @param data The data buffer to send.
     * @param len The length of the data buffer.
     */
    bool send_all(const char* data, size_t len);

    /**
     * Receives all data into the buffer from the server.
     * @param data The buffer to receive data into.
     * @param len The length of the data to receive.
     */
    bool receive_all(char* data, size_t len);

    int sock_fd;
};

#endif
