#include "../include/ServerConfig.h"

#include <cerrno>
#include <climits>
#include <cstdlib>
#include <stdexcept>
#include <string>
#include <vector>

ServerConfig::ServerConfig()
    : port(DEFAULT_SERVER_PORT), factory_id(0) {}

ServerConfig::ServerConfig(int port, int factory_id,
                           const std::vector<PeerInfo>& peers)
    : port(port), factory_id(factory_id), peers(peers) {}

static int parse_int_strict(const char* value,
                            const std::string& field_name) {
    if (!value || *value == '\0') {
        throw std::invalid_argument("Missing value for " + field_name);
    }

    char* end = nullptr;
    errno = 0;
    long parsed = std::strtol(value, &end, 10);

    if (errno != 0 || end == value || *end != '\0' ||
        parsed > INT_MAX || parsed < INT_MIN) {
        throw std::invalid_argument("Invalid integer for " + field_name +
                                    ": " + value);
    }

    return static_cast<int>(parsed);
}

static int parse_port(const char* value,
                      const std::string& field_name) {
    int port = parse_int_strict(value, field_name);
    if (port <= 0 || port > 65535) {
        throw std::invalid_argument("Port out of range for " + field_name +
                                    ": " + value);
    }
    return port;
}

// ./server <port> <factory_id> <num_peers> [<peer_id> <peer_ip> <peer_port>] ...
ServerConfig parse_server_config(int argc, char* argv[]) {
    if (argc < 4) {
        throw std::invalid_argument(
            std::string("Usage: ") + argv[0] +
            " <port> <factory_id> <num_peers> "
            "[<peer_id> <peer_ip> <peer_port>] ...");
    }

    const int port       = parse_port(argv[1], "server port");
    const int factory_id = parse_int_strict(argv[2], "factory_id");
    const int num_peers  = parse_int_strict(argv[3], "num_peers");

    if (factory_id < 0) {
        throw std::invalid_argument("factory_id must be non-negative");
    }

    if (num_peers < 0) {
        throw std::invalid_argument("num_peers must be non-negative");
    }

    const int expected_argc = 4 + num_peers * 3;
    if (argc != expected_argc) {
        throw std::invalid_argument(
            "Expected " + std::to_string(expected_argc - 1) +
            " arguments for " + std::to_string(num_peers) +
            " peer(s), got " + std::to_string(argc - 1));
    }

    std::vector<PeerInfo> peers;
    peers.reserve(num_peers);

    for (int i = 0; i < num_peers; ++i) {
        const int base = 4 + i * 3;

        PeerInfo peer;
        peer.id   = parse_int_strict(argv[base], "peer_id");
        peer.ip   = argv[base + 1];
        peer.port = parse_port(argv[base + 2], "peer_port");

        if (peer.id < 0) {
            throw std::invalid_argument("peer_id must be non-negative");
        }

        peers.push_back(std::move(peer));
    }

    return ServerConfig(port, factory_id, std::move(peers));
}