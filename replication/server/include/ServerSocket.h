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

    bool ConnectToPeers(const std::vector<PeerInfo>& peers);

    bool ReconnectPeer(int index, const PeerInfo& peer);

    bool IsPeerConnected(int index) const;

    bool IdentifyAsServer(int client_fd);

    bool ReceiveIdentification(int& type, int client_fd);

    bool SendAck(int client_fd);
    bool ReceiveAck(int client_fd);

    bool ReceiveAckFromPeer(int peer_index);

    bool Send(const Robot& robot, int client_fd = -1);
    bool Send(const CustomerRecord& record, int client_fd = -1);

    bool Receive(RobotOrder& order, int client_fd = -1);

    bool SendReplicationRequest(const ReplicationRequest& request,
                                int peer_index);

    bool ReceiveReplicationRequest(ReplicationRequest& request, int client_fd);

    int NumPeers() const { return peer_fds.size(); }

    void SetPeerFd(int index, int fd);

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