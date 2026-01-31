#include "../include/Robot.h"

Robot::Robot(int cust_id, int order_num, RobotType robot_t, int eng_id, int exp_id) {
    customer_id = cust_id;
    order_number = order_num;
    robot_type = robot_t;
    engineer_id = eng_id;
    expert_id = exp_id;
}