#ifndef CUSTOMERRECORDS_H
#define CUSTOMERRECORDS_H

#include <map>

struct CustomerRecord {
    int customer_id;
    int last_order;
};

class CustomerRecords {
   public:
    std::map<int, int> records;

    void update_record(int customer_id, int order_number);
    int get_last_order(int customer_id);
    CustomerRecord get_record(int customer_id);
};

#endif