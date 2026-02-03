#ifndef SERVERCONFIG_H
#define SERVERCONFIG_H

#include <string>

#include "../../common/include/Robot.h"

class ServerConfig {
   public:
    static const int DEFAULT_SERVER_PORT = 8080;

    ServerConfig();
    ServerConfig(int port, int* expert_engineers);
    int port;
    int expert_engineers;
};

ServerConfig parse_server_config(int argc, char* argv[]);

#endif