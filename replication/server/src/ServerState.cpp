#include "../include/ServerState.h"

ServerState::ServerState()
    : last_index(0), committed_index(0), primary_id(-1), factory_id(0) {}