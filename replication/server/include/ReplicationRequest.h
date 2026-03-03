#ifndef REPLICATIONREQUEST_H
#define REPLICATIONREQUEST_H

#include "../../common/include/StateMachineLog.h"

class ReplicationRequest {
public:
    ReplicationRequest();

    int factory_id;
    int committed_index;
    int last_index;
    MapOp operation;
};

#endif