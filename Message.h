#ifndef __MESSAGE_H__
#define __MESSAGE_H__

#include <string>

#include <string.h>


#define CLI_RAND_SEED       1234
#define SVR_RAND_SEED       4321

#define NUM_REQ             3
#define MAX_REQUESTER_LEN   32

#define REQUEST_BUF_SIZE    46
#define RESPONSE_BUF_SIZE   14

class Request {
    unsigned int req_id;             // Request id
    unsigned short user_id;          // User id
    std::string requester;  // Name with max 32 characters
    int num1;   
    int num2;

public:
    Request(): req_id(0), user_id(0), requester(""), 
        num1(0), num2(0) {}

    unsigned int GetReqID() { return req_id; }
    unsigned short GetUserID() { return user_id; }
    std::string GetRequester() { return requester; }
    int GetNum1() { return num1; }
    int GetNum2() { return num2; }

    void SetRequest(unsigned int req_id, unsigned short user_id, 
            std::string name, int num1, int num2);
    int Marshal(char *buffer);
    void Unmarshal(char *buffer);
    void PrintRequest();
};

class Response {
    unsigned int req_id;
    unsigned short user_id;
    unsigned int res_id;
    int sum;

public:
    Response(): req_id(0), user_id(0), res_id(0), sum(0) {}

    unsigned int GetReqID() { return req_id; }
    unsigned short GetUserID() { return user_id; }
    unsigned short GetResID() { return res_id; }
    int GetSum() { return sum; }

    void SetResponse(unsigned int req_id, unsigned short user_id, 
            unsigned int res_id, int sum);
    int Marshal(char *buffer);
    void Unmarshal(char *buffer);
    void PrintResponse();
};

#endif // end of #ifndef __MESSAGE_H__
