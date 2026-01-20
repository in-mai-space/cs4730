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
  // TODO: Write a code that marshals req_id, user_id
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

  // feel free to change "return 0;" below.
  return 0;
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

  // feel free to change "return 0;" below.
  return 0;
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
}

/******************************************************************************
    Part 3 - 2
******************************************************************************/

// This is the code going into the client of Part 3 - 2
void part_3_client_marshal_send_recv_unmarshal(Request &req, Response &res,
                                               char *buffer, int sockfd) {

  // TODO:
  // 1) get marshalled byte array (i.e., buffer) using Request (i.e., req).
  // 2) send the marshalled buffer over the network.
  // 3) wait and receive server's response.
  // 4) unmarshal the received byte array into Response (i.e., res).
}

// This is the code going into the server of Part 3 - 2
void part_3_server_recv_unmarshal(Request &req, char *buffer, int sockfd) {

  // TODO:
  // 1) receive the request in byte array using the socket from the client
  // 2) unmashal the received byte array into Request (i.e., res)
}

// This is the code going into the server of Part 3 - 2
void part_3_server_marshal_send(Response &res, char *buffer, int sockfd) {
  // TODO:
  // 1) get marshalled byte array (i.e., buffer) using Response (i.e., res).
  // 2) send the marshalled buffer over the network using send.
}
