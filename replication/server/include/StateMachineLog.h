#include <vector>

struct MapOp {
    int op_code;
    int arg1;
    int arg2;
};

class StateMachineLog {
   public:
    std::vector<MapOp> log;

    // Append a new entry (used by PFA).
    void add_operation(int op_code, int arg1, int arg2);

    // Write an entry at a specific 1-based index (used by IFA).
    // Resizes the log if necessary.
    void write_operation(int index, int op_code, int arg1, int arg2);

    // Read the entry at a specific 1-based index.
    MapOp get_operation(int index) const;
};