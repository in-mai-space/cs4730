#include "../include/CustomerRecords.h"

void CustomerRecords::UpdateRecord(int customer_id, int order_number) {
    records[customer_id] = order_number;
}

int CustomerRecords::GetLastOrder(int customer_id) {
    if (records.find(customer_id) != records.end()) {
        return records[customer_id];
    }
    return -1;
}

CustomerRecord CustomerRecords::GetRecord(int customer_id) {
    auto it = records.find(customer_id);
    if (it != records.end()) {
        return {customer_id, it->second};
    }
    return {-1, -1};
}