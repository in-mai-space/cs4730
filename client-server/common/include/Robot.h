#ifndef ROBOT_H
#define ROBOT_H

class Robot {
    public:
        int customer_id; 
        int order_number; 
        int robot_type; 
        int engineer_id; 
        int expert_id;

        Robot(int cust_id, int order_num, int robot_t, int eng_id, int exp_id);
};

#endif