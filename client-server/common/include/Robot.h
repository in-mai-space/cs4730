#ifndef ROBOT_H
#define ROBOT_H

class Robot {
   public:
    int customer_id;
    int order_number;
    int robot_type; // 0 or 1
    int engineer_id;
    int expert_id;

    /**
     * Constructor to initialize all fields of the Robot.
     * @param cust_id The customer ID.
     * @param order_num The order number.
     * @param robot_t The robot type.
     * @param eng_id The engineer ID.
     * @param exp_id The expert ID.
     */
    Robot(int cust_id, int order_num, int robot_t, int eng_id, int exp_id);
};

#endif