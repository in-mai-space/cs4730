#ifndef SERVERSTUB_H
#define SERVERSTUB_H
#include "../../common/include/Robot.h"
#include "../../common/include/RobotOrder.h"
#include "ServerSocket.h"
#include <queue>
#include <future>

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
     * @param engineer_id The ID of the engineer handling the client.
     */
    void handle_client(RobotOrder& order, int client_fd, int engineer_id, std::shared_ptr<std::queue<std::promise<Robot>>>& jobQueue, std::mutex &mtx, std::condition_variable &cv);

    /**
     * Processes a RobotOrder and generates a Robot response.
     * @param order The RobotOrder received from the client.
     * @param engineer_id The ID of the engineer processing the order.
     */
    Robot process_order(const RobotOrder& order, int engineer_id);

    /**
     * Attaches a special module to the given Robot.
     * @param promise The promise to fulfill with the modified robot.
     * @param expert_id The ID of the expert engineer attaching the module.
     */
    void attach_special_module(std::promise<Robot>&& promise, int expert_id);

   private:
    ServerSocket* socket;
    bool is_special_robot(int robot_type);
};

#endif