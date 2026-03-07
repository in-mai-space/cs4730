#ifndef SERVERSOCKET_H
#define SERVERSOCKET_H
#include <string>
#include <vector>

#include "../../common/include/CustomerRecords.h"
#include "../../common/include/Robot.h"
#include "../../common/include/RobotOrder.h"
#include "./ReplicationRequest.h"
#include "./ServerConfig.h"

class ServerSocket {
   public:
    ServerSocket();

    bool listen(int port);
    int accept();

    bool connect_to_peers(const std::vector<PeerInfo>& peers);

    // Reconnect a single peer (used when a failed server restarts).
    bool reconnect_peer(int index, const PeerInfo& peer);

    bool is_peer_connected(int index) const;

    bool identify_as_server(int client_fd);

    bool receive_identification(int& type, int client_fd);

    bool send_ack(int client_fd);
    bool receive_ack(int client_fd);

    bool receive_ack_from_peer(int peer_index);

    bool send(const Robot& robot, int client_fd = -1);
    bool send(const CustomerRecord& record, int client_fd = -1);

    bool receive(RobotOrder& order, int client_fd = -1);

    bool send_replication_request(const ReplicationRequest& request,
                                  int peer_index);

    bool receive_replication_request(ReplicationRequest& request,
                                     int client_fd);

    int num_peers() const { return peer_fds.size(); }

    void set_peer_fd(int index, int fd);

    int get_peer_index(int fd);

   private:
    int socket_fd;
    int client_fd;

    std::vector<int> peer_fds;

    bool send_all(const char* data, size_t len, int fd);
    bool receive_all(char* data, size_t len, int fd);

    int marshall(const Robot& robot, char* buffer, int size);
    int marshall(const CustomerRecord& record, char* buffer, int size);
    int marshall(const ReplicationRequest& request, char* buffer, int size);

    int unmarshall(const char* buffer, int size, RobotOrder& order);
    int unmarshall(const char* buffer, int size, ReplicationRequest& req);
};

#endif