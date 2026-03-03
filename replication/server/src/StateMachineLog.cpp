#include "../include/StateMachineLog.h"

void StateMachineLog::add_operation(int op_code, int arg1, int arg2) {
    log.push_back({op_code, arg1, arg2});
}

void StateMachineLog::write_operation(int index, int op_code, int arg1,
                                      int arg2) {
    if (index > (int)log.size()) {
        log.resize(index, {0, 0, 0});
    }
    log[index - 1] = {op_code, arg1, arg2};
}

MapOp StateMachineLog::get_operation(int index) const {
    return log[index - 1];
}