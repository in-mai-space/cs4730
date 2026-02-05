#ifndef SERVERSTUB_H
#define SERVERSTUB_H
#include "../../common/include/Robot.h"
#include "../../common/include/RobotOrder.h"
#include "ServerSocket.h"
#include <queue>
#include <future>

struct ExpertRequestQueue {
    std::queue<std::promise<Robot>> jobQueue;
    std::mutex mtx;
    std::condition_variable cv;
};

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
     * @param expertQueue The shared expert request queue.
     */
    void handle_client(int client_fd, int engineer_id, ExpertRequestQueue& expertQueue);

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

    /**
     * Receives an order from the client through the socket.
     * @param order The RobotOrder object to populate.
     * @param client_fd The file descriptor of the connected client.
     * @return true if successful, false otherwise.
     */
    bool receive_order(RobotOrder& order, int client_fd);

    /**
     * Ships a robot to the client through the socket.
     * @param robot The Robot object to send.
     * @param client_fd The file descriptor of the connected client.
     * @return true if successful, false otherwise.
     */
    bool ship_robot(const Robot& robot, int client_fd);

   private:
    ServerSocket* socket;
    bool is_special_robot(int robot_type);
};

#endif