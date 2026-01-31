#ifndef ROBOT_H
#define ROBOT_H

enum RobotType {
    SPECIAL = 1,
    REGULAR = 0
};

class Robot {
    public:
        int customer_id; 
        int order_number; 
        RobotType robot_type; 
        int engineer_id; 
        int expert_id;

        Robot(int cust_id, int order_num, RobotType robot_t, int eng_id, int exp_id);
};

#endif