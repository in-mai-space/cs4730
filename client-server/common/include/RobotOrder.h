#ifndef ORDER_H
#define ORDER_H
#include "Robot.h"

class RobotOrder {
	public:
		int customer_id;
		int order_number;
		RobotType robot_type;

		RobotOrder(int cust_id, int order_num, RobotType robot_t);
};

#endif