#include <vector>

struct MapOp {
    int op_code;
    int arg1;
    int arg2;
};

class StateMachineLog {
   public:
    std::vector<MapOp> log;

    void add_operation(int op_code, int arg1, int arg2);
};