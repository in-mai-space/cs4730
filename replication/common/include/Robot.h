#ifndef ROBOT_H
#define ROBOT_H

class Robot {
   public:
    int customer_id;
    int order_number;
    int request_type;  // 0 or 1
    int engineer_id;
    int admin_id;

    /**
     * Constructor to initialize all fields of the Robot.
     * @param cust_id The customer ID.
     * @param order_num The order number.
     * @param request_type The request type.
     * @param eng_id The engineer ID.
     * @param admin_id The admin ID.
     */
    Robot(int cust_id, int order_num, int request_type, int eng_id,
          int admin_id);
};

#endif