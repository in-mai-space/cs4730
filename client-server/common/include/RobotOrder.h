#ifndef ORDER_H
#define ORDER_H
#include "Robot.h"

class RobotOrder {
	public:
		int customer_id;
		int order_number;
		int robot_type;

		RobotOrder(int cust_id, int order_num, int robot_t);
};

#endif