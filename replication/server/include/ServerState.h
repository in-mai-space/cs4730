#ifndef SERVERSTATE_H
#define SERVERSTATE_H

class ServerState {
    public:
     int last_index;
     int committed_index;
     int primary_id;
     int factory_id;
};

#endif