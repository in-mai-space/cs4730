#ifndef SERVERCONFIG_H
#define SERVERCONFIG_H

#include <string>

#include "../../common/include/Robot.h"

class ServerConfig {
   public:
    static const int DEFAULT_SERVER_PORT = 8080;

    /**
     * Default constructor initializing with default values.
     */
    ServerConfig();

    /**
     * Parameterized constructor to initialize all configuration fields.
     * @param port The server port number.
     * @param expert_engineers Pointer to the number of expert engineers.
     */
    ServerConfig(int port, int* expert_engineers);
    int port;
    int expert_engineers;
};

/**
 * Parses command line arguments to create a ServerConfig object.
 * Expects the following arguments:
 * argv[1]: server_port (int)
 * argv[2]: [num_expert_engineers] (int, optional)
 *
 * Example: ./server 12345 2
 */
ServerConfig parse_server_config(int argc, char* argv[]);

#endif