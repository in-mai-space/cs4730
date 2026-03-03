#ifndef SERVERCONFIG_H
#define SERVERCONFIG_H

#include <string>
#include <vector>

// Info for one peer factory server
struct PeerInfo {
    int id;
    std::string ip;
    int port;
};

class ServerConfig {
   public:
    static const int DEFAULT_SERVER_PORT = 8080;

    ServerConfig();
    ServerConfig(int port, int factory_id, const std::vector<PeerInfo>& peers);

    int port;                     // this factory's listen port
    int factory_id;               // unique ID of this factory (>= 0)
    std::vector<PeerInfo> peers;  // peer factory servers
};

/**
 * Parses command line arguments to create a ServerConfig object.
 *
 * Format:
 *   ./server <port> <factory_id> <num_peers>
 *            [<peer_id> <peer_ip> <peer_port>] ...
 *
 * Example (factory 0, two peers):
 *   ./server 12345 0 2  1 127.0.0.1 12346  2 127.0.0.1 12347
 */
ServerConfig parse_server_config(int argc, char* argv[]);

#endif