#ifndef CLIENTCONFIG_H
#define CLIENTCONFIG_H

#include <string>

#include "../../common/include/Robot.h"

class ClientConfig {
    public:
        static const int DEFAULT_SERVER_PORT = 8080;
        static const std::string DEFAULT_SERVER_IP;

        ClientConfig();
        ClientConfig(std::string server_ip, int server_port, int customers, int orders, int robot_type);

        std::string server_ip;
        int server_port;
        int customers;
        int orders;
        int robot_type;
};

ClientConfig parse_client_config(int argc, char *argv[]);

#endif