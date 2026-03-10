#ifndef CUSTOMERRECORDS_H
#define CUSTOMERRECORDS_H

#include <map>

struct CustomerRecord {
    int customer_id;
    int last_order;
};

class CustomerRecords {
   public:
    /**
     * Updates the record for a given customer ID with the new order number.
     * If the customer ID does not exist, it will be created.
     * @param customer_id The ID of the customer whose record to update.
     * @param order_number The new order number to set for the customer.
     */
    void UpdateRecord(int customer_id, int order_number);

    /**
     * Retrieves the last order number for a given customer ID. If the customer
     * ID does not exist, it returns -1.
     * @param customer_id The ID of the customer
     */
    int GetLastOrder(int customer_id);

    /**
     * Retrieves the CustomerRecord for a given customer ID. If the customer ID
     * does not exist, it returns a record with customer_id and last_order set
     * to -1.
     * @param customer_id The ID of the customer
     */
    CustomerRecord GetRecord(int customer_id);

   private:
    std::map<int, int> records;  // maps customer_id to last_order
};

#endif