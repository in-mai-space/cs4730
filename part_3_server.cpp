#include <iostream>

#include <assert.h>
#include <string.h>
#include <unistd.h>

#include <arpa/inet.h>
#include <net/if.h>
#include <netdb.h>
#include <netinet/tcp.h>
#include <sys/types.h>
#include <sys/socket.h>

#include "Message.h"

#define BACKLOG	8
#define BUFFER_SIZE 64

// Prototypes of what you will implement in part_3_your_task.cpp
void part_3_server_recv_unmarshal(Request &req, char *buffer, int sockfd);
void part_3_server_marshal_send(Response &res, char *buffer, int sockfd);


// This main function takes one command line arguments:
// port that the server will accept connections from the clients
int main(int argc, char *argv[]) {
	int sockfd, newfd;
	struct sockaddr_in my_addr, addr;
	unsigned int addr_size = sizeof(addr);
	int port;
	int length;
	char buffer[BUFFER_SIZE];

	if (argc < 2) {
		std::cout << "Usage\n" << argv[0] << " [port]"<< std::endl;
		return 0;
	}
    
	port = atoi(argv[1]);

/******************************************************************************
 * Initializing the network connection
******************************************************************************/

    // Initialize the socekt and basic network info
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0) {
		perror("ERROR: failed to create socket");
		return 0;
	}
	memset(&my_addr, '\0', sizeof(my_addr));
	my_addr.sin_family = AF_INET;
	my_addr.sin_port = htons(port);
	my_addr.sin_addr.s_addr = INADDR_ANY;

    // Bind the socket to the network info
	if ((bind(sockfd, (struct sockaddr *) &my_addr, sizeof(my_addr))) < 0) {
		perror("ERROR: failed to bind");
		return 0;
	}

    // Listen to incoming connections
	listen(sockfd, BACKLOG);

    std::cout << "Waiting for client to connect." << std::endl;

    // Accept new connections
	newfd = accept(sockfd, (struct sockaddr *) &addr, &addr_size);
	if (newfd < 0) {
		perror("ERROR: failed to accept");
		return 0;
	}

/******************************************************************************
 * This is a sample send receive code
******************************************************************************/

    // Wait for and receive client message
    // This is a blocking call: i.e., your program
    // will wait here until client sends a message.
	length = recv(newfd, buffer, 11, 0);
	if (length < 0) {
		perror("ERROR: failed to recv");
		return 0;
	}

    std::cout << "Received: " << buffer << "\n" << std::endl;

    // Send back some messages to the client
	length = send(newfd, "Hi client!\0", 11, 0);
	if (length < 0) {
		perror("ERROR: failed to send");
		return 0;
	}

/******************************************************************************
 * Directly relevant code section to Part 3
******************************************************************************/


    // Random seed setting
    std::srand(SVR_RAND_SEED);

    for (int i = 0; i < NUM_REQ; i++) {
        Request req;
        Response res;

        int sum;

        // response id is randomly generated
        unsigned int res_id = std::rand(); 

        // TODO: complete this function in part_3_your_task.cpp.
        // You should receive and unmarshall the incoming request
        // so that the output below shows the same output as
        // what the client sent. 
        part_3_server_recv_unmarshal(req, buffer, newfd);
        
        std::cout << "[Received] ";
        req.PrintRequest();

        // The server's main role is to compute 
        // the sum of two received numbers.
        sum = req.GetNum1() + req.GetNum2();

        // Set response based on the request and sum and print
        res.SetResponse(req.GetReqID(), req.GetUserID(), res_id, sum);
        std::cout << "[Sending ] ";
        res.PrintResponse();

        // TODO: complete this function in part_3_your_task.cpp.
        // This function should marshal what is in res into
        // buffer and send it over the network.
        part_3_server_marshal_send(res, buffer, newfd);

    }

   	close(sockfd);
	return 1;
}
