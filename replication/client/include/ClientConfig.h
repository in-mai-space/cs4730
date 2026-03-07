#ifndef CLIENTCONFIG_H
#define CLIENTCONFIG_H

#include <string>

#include "../../common/include/Robot.h"

class ClientConfig {
   public:
    static const int DEFAULT_SERVER_PORT = 8080;
    static const std::string DEFAULT_SERVER_IP;

    /**
     * Default constructor initializing with default values.
     */
    ClientConfig();

    /**
     * Parameterized constructor to initialize all configuration fields.
     * @param server_ip The server IP address.
     * @param server_port The server port number.
     * @param customers The number of customers.
     * @param orders The number of orders per customer.
     * @param request_type The type of request (1, 2, or 3).
     */
    ClientConfig(std::string server_ip, int server_port, int customers,
                 int orders, int request_type);

    std::string server_ip;
    int server_port;
    int customers;
    int orders;
    int request_type;
};

/**
 * Parses command line arguments to create a ClientConfig object.
 * Expects the following arguments:
 * argv[1]: server_ip (string)
 * argv[2]: server_port (int)
 * argv[3]: num_customers (int)
 * argv[4]: num_orders (int)
 * argv[5]: request_type (int 1, 2, or 3)
 *
 * Example: ./client 123.456.789.123 12345 16 1000 2
 */
ClientConfig parse_and_validate_client_config(int argc, char* argv[]);

#endif