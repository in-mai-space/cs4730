#include <vector>

struct MapOp {
    int op_code;
    int arg1;
    int arg2;
};

class StateMachineLog {
   public:
    /**
     * Appends a new operation to the end of the log. Returns the 1-based index
     * of the new entry.
     * @param op_code The operation code (e.g., 1 for order).
     * @param arg1 The first argument (e.g., customer ID).
     * @param arg2 The second argument (e.g., order number).
     */
    void AddOperation(int op_code, int arg1, int arg2);

    /**
     * Writes an operation at a specific 1-based index. This is used by the IFA
     * protocol.
     * @param index The 1-based index where the operation should be written.
     * @param op_code The operation code (e.g., 1 for order).
     * @param arg1 The first argument (e.g., customer ID).
     * @param arg2 The second argument (e.g., order number).
     */
    void WriteOperation(int index, int op_code, int arg1, int arg2);

    /**
     * Reads an operation at a specific 1-based index.
     * @param index The 1-based index of the operation to read.
     * @return The operation at the specified index.
     */
    MapOp GetOperation(int index) const;

   private:
    std::vector<MapOp> log;
};