#include "../include/StateMachineLog.h"

void StateMachineLog::add_operation(int op_code, int arg1, int arg2) {
    log.push_back({op_code, arg1, arg2});
}