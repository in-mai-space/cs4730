#ifndef SERVERSOCKET_H
#define SERVERSOCKET_H 

class ServerSocket {
    public:
        bool listen(int port);
        int accept();
};

#endif