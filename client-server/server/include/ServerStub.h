#ifndef SERVERSTUB_H
#define SERVERSTUB_H
#include "../../common/include/Robot.h"
#include "../../common/include/RobotOrder.h"
#include "ServerSocket.h"

class ServerStub {
   public:  
    /**
     * Initializes the server stub with the given server socket.
     * @param socket Pointer to the ServerSocket instance.
     */
    void init(ServerSocket* socket);

    /**
     * Handles client requests in a loop until the client disconnects.
     * @param client_fd The file descriptor of the connected client.
     */
    void handle_client(int client_fd);

    /**
     * Processes a RobotOrder and generates a Robot response.
     * @param order The RobotOrder received from the client.
     */
    Robot process_order(const RobotOrder& order);

   private:
    ServerSocket* socket;
};

#endif