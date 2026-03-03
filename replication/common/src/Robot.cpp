#include "../include/Robot.h"

Robot::Robot(int cust_id, int order_num, int req_type, int eng_id, int adm_id) {
    customer_id = cust_id;
    order_number = order_num;
    request_type = req_type;
    engineer_id = eng_id;
    admin_id = adm_id;
}