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

/**
 * Parses command line arguments to create a ClientConfig object.
 * Expects the following arguments:
 * argv[1]: server_ip (string)
 * argv[2]: server_port (int)
 * argv[3]: num_customers (int)
 * argv[4]: num_orders (int)
 * argv[5]: robot_type (int 0 or 1)
 * 
 * Example: ./client 123.456.789.123 12345 16 1000 0
 */
ClientConfig parse_and_validate_client_config(int argc, char *argv[]);

#endif