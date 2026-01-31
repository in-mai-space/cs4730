#include "../include/RobotOrder.h"

RobotOrder::RobotOrder(int cust_id, int order_num, RobotType robot_t) {
    customer_id = cust_id;
    order_number = order_num;
    robot_type = robot_t;
}