#ifndef SERVERSOCKET_H
#define SERVERSOCKET_H 

class ServerSocket {
    public:
        bool Listen(int port);
        int Accept();
};

#endif