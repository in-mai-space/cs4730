#include <iostream>
#include <string>

#include <string.h>
#include <arpa/inet.h>

#include "Message.h"

void Request::SetRequest(unsigned int req_id, unsigned short user_id, 
        std::string name, int num1, int num2) {
    this->req_id = req_id;
    this->user_id = user_id;
    requester = name;
    this->num1 = num1;
    this->num2 = num2;
}

void Request::PrintRequest() {
    std::cout << "\tREQUEST - req_id: " << req_id << "; requester id: " << user_id;
    std::cout << "; requester: " << requester << "; ";
    std::cout << "num1: " << num1;
    std::cout << "; num2: " << num2 << std::endl;
}

void Response::SetResponse(unsigned int req_id, unsigned short user_id, 
        unsigned int res_id, int sum) {
    this->req_id = req_id;
    this->user_id = user_id;
    this->res_id = res_id;
    this->sum = sum;
}

void Response::PrintResponse() {
    std::cout << "\tRESPONSE - req_id: " << req_id << "; user_id: " << user_id;
    std::cout << "; res_id: " << res_id << "; sum: " << sum << std::endl;
}


