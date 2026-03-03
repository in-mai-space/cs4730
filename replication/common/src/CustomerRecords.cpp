#include "../include/CustomerRecords.h"

void CustomerRecords::update_record(int customer_id, int order_number) {
    records[customer_id] = order_number;
}

int CustomerRecords::get_last_order(int customer_id) {
    if (records.find(customer_id) != records.end()) {
        return records[customer_id];
    }
    return -1;
}

CustomerRecord CustomerRecords::get_record(int customer_id) {
    auto it = records.find(customer_id);
    if (it != records.end()) {
        return {customer_id, it->second};
    }
    return {-1, -1};
}