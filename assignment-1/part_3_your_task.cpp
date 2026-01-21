#include <iostream>
#include <string>

#include <arpa/inet.h>
#include <assert.h>
#include <string.h>

#include "Message.h"

/******************************************************************************
    Part 3 - 1
******************************************************************************/

// This is a function that the client program will use
// before sending the request to the server.
// The argument, buffer, is a C-string or a character array
// that needs to be filled-in in this function body.
// TODO: return value should match with the amount of
// data encoded into the buffer.
int Request::Marshal(char *buffer) {
  // TODO: Write a code that marshals req_id, user_id,
  //  requester (32 character string), num1, and
  //  num2 into a C-string (i.e., buffer) for the
  //  transfer over the network. Note that (unsigned) int and short
  //  types need to be encoded into network format
  //  using hton functions (use htons or htonl depending on
  //  the size of data). Requester only needs to be converted
  //  to a C-string and does not need to be network encoded,
  //  but make sure the string is a null terminating
  //  C-string (i.e., ends with '\0' character) .
  //
  //  The format should be
  //  [req_id 4B][user_id 2B][requester 32B][num1 4B][num2 4B].
  //  You will need to use memcpy to copy the data to the buffer.
  constexpr size_t REQUESTER_SIZE = 32;
  size_t offset = 0;
  uint32_t net_req_id = htonl(req_id);
  uint16_t net_user_id = htons(user_id);
  uint32_t net_num1 = htonl(num1);
  uint32_t net_num2 = htonl(num2);

  std::memcpy(buffer + offset, &net_req_id, sizeof(net_req_id));
  offset += sizeof(net_req_id);

  std::memcpy(buffer + offset, &net_user_id, sizeof(net_user_id));
  offset += sizeof(net_user_id);

  std::memset(buffer + offset, 0, REQUESTER_SIZE);
  std::memcpy(buffer + offset, requester.c_str(),
              std::min(requester.size(), REQUESTER_SIZE - 1));
  offset += REQUESTER_SIZE;

  std::memcpy(buffer + offset, &net_num1, sizeof(net_num1));
  offset += sizeof(net_num1);

  std::memcpy(buffer + offset, &net_num2, sizeof(net_num2));
  offset += sizeof(net_num2);

  return offset;
}

// This is a function that the server program will use
// after receiving the request from the client to
// parse the Message content out of the C-string.
// The argument, buffer, is a C-string or a character array
// which contains the marshalled data.
void Request::Unmarshal(char *buffer) {
  // TODO: unmarshal input C-string (buffer) and assign the
  // retrieved values to the member variables of this class.
  // Input buffer format should be
  // [req_id 4B][user_id 2B][requester 32B][num1 4B][num2 4B].
  // (unsigned) int and short types are in network format so you will need to
  // convert them to host format using ntoh functions (use ntons or ntonl
  // depending on the size of data). Requester only needs to be converted
  // to std::string and does not need to be converted to the host format.
  constexpr size_t REQUESTER_SIZE = 32;
  size_t offset = 0;

  uint32_t net_req_id;
  uint16_t net_user_id;
  uint32_t net_num1;
  uint32_t net_num2;

  std::memcpy(&net_req_id, buffer + offset, sizeof(net_req_id));
  offset += sizeof(net_req_id);
  req_id = ntohl(net_req_id);

  std::memcpy(&net_user_id, buffer + offset, sizeof(net_user_id));
  offset += sizeof(net_user_id);
  user_id = ntohs(net_user_id);

  requester =
      std::string(buffer + offset, strnlen(buffer + offset, REQUESTER_SIZE));
  offset += REQUESTER_SIZE;

  std::memcpy(&net_num1, buffer + offset, sizeof(net_num1));
  offset += sizeof(net_num1);
  num1 = ntohl(net_num1);

  std::memcpy(&net_num2, buffer + offset, sizeof(net_num2));
  offset += sizeof(net_num2);
  num2 = ntohl(net_num2);
}

// This is a function that the server program will use
// before sending the response to the client.
// The argument, buffer, is a C-string or a character array
// that needs to be filled-in in this function body.
// TODO: return value should match with the amount of
// data encoded into the buffer.
int Response::Marshal(char *buffer) {
  // TODO: Write a code that marshals req_id, user_id,
  //  and sum into a C-string (i.e., buffer) for the
  //  transfer over the network. Note that (unsigned) int and short
  //  types need to be encoded into network format
  //  using hton variant functions (use htons or htonl depending on
  //  the size of data).
  //  The format should be
  //  [req_id 4B][user_id 2B][res_id 4B][response 4B].
  size_t offset = 0;
  uint32_t net_req_id = htonl(req_id);
  uint16_t net_user_id = htons(user_id);
  uint32_t net_res_id = htonl(res_id);
  uint32_t net_response = htonl(sum);

  std::memcpy(buffer + offset, &net_req_id, sizeof(net_req_id));
  offset += sizeof(net_req_id);

  std::memcpy(buffer + offset, &net_user_id, sizeof(net_user_id));
  offset += sizeof(net_user_id);

  std::memcpy(buffer + offset, &net_res_id, sizeof(net_res_id));
  offset += sizeof(net_res_id);

  std::memcpy(buffer + offset, &net_response, sizeof(net_response));
  offset += sizeof(net_response);

  return offset;
}

// This is a function that the client program will use
// after receiving the response from the server.
// Parse the Message content out of the C-string.
// The argument, buffer, is a C-string or a character array
// which contains the marshalled data.
void Response::Unmarshal(char *buffer) {
  // TODO: unmarshal input C-string (buffer) and assign the
  // retrieved values to the member variables of this class.
  // Input buffer format should be
  // [req_id 4B][user_id 2B][res_id 4B][response 4B].
  // (unsigned) int and short types are in network format so you will need to
  // convert them to host format using ntoh functions (use ntons or ntonl
  // depending on the size of data).
  size_t offset = 0;

  uint32_t net_req_id;
  uint16_t net_user_id;
  uint32_t net_res_id;
  uint32_t net_response;

  std::memcpy(&net_req_id, buffer + offset, sizeof(net_req_id));
  offset += sizeof(net_req_id);
  req_id = ntohl(net_req_id);

  std::memcpy(&net_user_id, buffer + offset, sizeof(net_user_id));
  offset += sizeof(net_user_id);
  user_id = ntohs(net_user_id);

  std::memcpy(&net_res_id, buffer + offset, sizeof(net_res_id));
  offset += sizeof(net_res_id);
  res_id = ntohl(net_res_id);

  std::memcpy(&net_response, buffer + offset, sizeof(net_response));
  offset += sizeof(net_response);
  sum = ntohl(net_response);
}

/******************************************************************************
    Part 3 - 2
******************************************************************************/

// This is the code going into the client of Part 3 - 2
void part_3_client_marshal_send_recv_unmarshal(Request &req, Response &res,
                                               char *buffer, int sockfd) {
  // get marshalled byte array using Request
  int request_size = req.Marshal(buffer);

  // streaming the request
  int total_sent = 0;
  while (total_sent < request_size) {
    int sent = send(sockfd, buffer + total_sent, request_size - total_sent, 0);
    total_sent += sent;
  }

  // wait and get streamed response
  constexpr int SERVER_RESPONSE_SIZE = 14;
  int total_received = 0;

  while (total_received < SERVER_RESPONSE_SIZE) {
    int received = recv(sockfd, buffer + total_received,
                        SERVER_RESPONSE_SIZE - total_received, 0);
    total_received += received;
  }

  // unmarshal the received byte array into Response
  res.Unmarshal(buffer);
}

// This is the code going into the server of Part 3 - 2
void part_3_server_recv_unmarshal(Request &req, char *buffer, int sockfd) {
  // receive the request in byte array using socket
  constexpr int CLIENT_REQUEST_SIZE = 46;
  int total_received = 0;

  while (total_received < CLIENT_REQUEST_SIZE) {
    int received = recv(sockfd, buffer + total_received,
                        CLIENT_REQUEST_SIZE - total_received, 0);
    total_received += received;
  }

  // unmarshall the received byte array into Request
  req.Unmarshal(buffer);
}

// This is the code going into the server of Part 3 - 2
void part_3_server_marshal_send(Response &res, char *buffer, int sockfd) {
  // get marshalled byte array
  int response_size = res.Marshal(buffer);

  // send the marshalled buffer over network using send
  int total_sent = 0;
  while (total_sent < response_size) {
    int sent_size =
        send(sockfd, buffer + total_sent, response_size - total_sent, 0);
    total_sent += sent_size;
  }
}
