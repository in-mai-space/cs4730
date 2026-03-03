#ifndef SERVERSOCKET_H
#define SERVERSOCKET_H
#include <cstddef>

#include "../../common/include/CustomerRecords.h"
#include "../../common/include/Robot.h"
#include "../../common/include/RobotOrder.h"
#include "./ServerConfig.h"
#include "./ReplicationRequest.h"

class ServerSocket {
   public:
    ServerSocket();

    /**
     * Sets up the server to listen on the specified port.
     * @param port The port number to listen on.
     */
    bool listen(int port);

    /**
     * Accepts a new client connection.
     * @return The file descriptor of the accepted client socket.
     */
    int accept();

    /**
     * Establish connections with all peer nodes.
     * Sends identification type=1 (PFA) on each connection.
     */
    bool connect_to_peers(const std::vector<PeerInfo>& peers);

    /**
     * Returns the number of connected peer nodes.
     */
    int num_peers() const { return (int)peer_fds.size(); }

    /**
     * Sends an identification message on the given fd.
     * type 0 = customer, type 1 = PFA/replication.
     */
    bool send_identification(int type, int client_fd);

    /**
     * Receives an identification message from the given fd.
     */
    bool receive_identification(int& type, int client_fd);

    /**
     * Sends a one-int acknowledgement to the given fd.
     */
    bool send_ack(int client_fd);

    /**
     * Receives a one-int acknowledgement from the given fd.
     */
    bool receive_ack(int client_fd);

    /**
     * Receives an ack from the peer at peer_index in peer_fds (used by PFA).
     */
    bool receive_ack_from_peer(int peer_index);

    /**
     * Sends a Robot to the specified client.
     * @param robot The Robot to send.
     * @param client_fd The file descriptor of the client socket.
     */
    bool send(const Robot& robot, int client_fd = -1);

    /**
     * Sends a CustomerRecord to the specified client.
     * @param record The CustomerRecord to send.
     * @param client_fd The file descriptor of the client socket.
     */
    bool send(const CustomerRecord& record, int client_fd = -1);

    /**
     * Send a replication request to the peer at peer_index in peer_fds.
     */
    bool send_replication_request(const ReplicationRequest& request,
                                  int peer_index);

    /**
     * Receive replication request from an IFA-connected client.
     */
    bool receive_replication_request(ReplicationRequest& request,
                                     int client_fd);

    /**
     * Receives a RobotOrder from the specified client.
     * @param order The RobotOrder object to populate with received data.
     * @param client_fd The file descriptor of the client socket.
     */
    bool receive(RobotOrder& order, int client_fd = -1);

    /**
     * Sets the client file descriptor for communication.
     * @param client_fd The file descriptor of the client socket.
     */
    void set_client_fd(int client_fd) { this->client_fd = client_fd; }

   private:
    /**
     * Marshalls a Robot into a byte buffer.
     */
    int marshall(const Robot& robot, char* buffer, int buffer_size);

    /**
     * Marshalls a ReplicationRequest into a byte buffer.
     */
    int marshall(const ReplicationRequest& request, char* buffer, int buffer_size);

    /**
     * Marshalls a CustomerRecord into a byte buffer.
     */
    int marshall(const CustomerRecord& record, char* buffer, int buffer_size);

    /**
     * Unmarshalls a byte buffer in a CustomerRecord.
     */
    int unmarshall(const char* buffer, int buffer_size, CustomerRecord& record);

    /**
     * Unmarshalls a byte buffer into a RobotOrder.
     */
    int unmarshall(const char* buffer, int buffer_size, RobotOrder& order);

    /**
     * Unmarshalls a byte buffer into a ReplicationRequest.
     */
    int unmarshall(const char* buffer, int buffer_size, ReplicationRequest& request);

    /**
     * Sends all data in the buffer to the specified client.
     * @param data The data buffer to send.
     * @param len The length of the data buffer.
     * @param client_fd The file descriptor of the client socket.
     */
    bool send_all(const char* data, size_t len, int client_fd);

    /**
     * Receives all data into the buffer from the specified client.
     * @param data The buffer to receive data into.
     * @param len The length of the data to receive.
     * @param client_fd The file descriptor of the client socket.
     */
    bool receive_all(char* data, size_t len, int client_fd);

    int socket_fd;
    int client_fd;
    std::vector<int> peer_fds;
};

#endif