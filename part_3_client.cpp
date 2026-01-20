#include <iostream>

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <arpa/inet.h>
#include <net/if.h>
#include <netdb.h>
#include <netinet/tcp.h>
#include <sys/types.h>
#include <sys/socket.h>

#include "Message.h"

#define BUFFER_SIZE 64

#define SHORT_MAX   65536
#define NUM_MAX     10000000

// Prototype of what you will implement in part_3_your_task.cpp
void part_3_client_marshal_send_recv_unmarshal(Request &req, Response &res, 
        char *buffer, int sockfd);


// This main function takes command two line arguments:
// server's IP address and port
int main(int argc, char *argv[]) {
	int sockfd;
	struct sockaddr_in addr;
	int port;
	int length;
	char buffer[BUFFER_SIZE];

	if (argc < 3) {
		std::cout << "Usage\n" << argv[0] << " [ip] [port]"<< std::endl;
		return 0;
	}
    // Convert the second command line argument to int to get 
    // the port number of the server
	port = atoi(argv[2]);

/******************************************************************************
 * Initializing the network connection
******************************************************************************/

    // Set up the socket and connection info
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0) {
		perror("ERROR: failed to create socket");
		return 0;
	}
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
    // Use the first command line argument which is the IP 
    // address of the server
	addr.sin_addr.s_addr = inet_addr(argv[1]);

    // Connect to the server
	if (connect(sockfd, (struct sockaddr *) &addr, sizeof(addr)) < 0) {
		perror("ERROR: failed to connect");
		return 0;
	}

/******************************************************************************
 * This is a sample send receive code
******************************************************************************/
    // Send a message to the server    
	length = send(sockfd, "Hi server!\0", 11, 0);
	if (length < 0) {
		perror("ERROR: failed to send");
		return 0;
	}

    // Wait for and receive message from the server
    // This is a blocking call: i.e., your program
    // will wait here until the server sends a message.
	length = recv(sockfd, buffer, 11, 0);
	if (length < 0) {
		perror("ERROR: failed to recv");
		return 0;
	}

    std::cout << "Received: " << buffer << "\n" << std::endl;


/******************************************************************************
 * Directly relevant code section to Part 3
******************************************************************************/
    // Random seed setting
    std::srand(CLI_RAND_SEED);


    for (unsigned int i = 0; i < NUM_REQ; i++) {
        Request req;
        Response res;
       
        unsigned short user_id = abs(std::rand() % SHORT_MAX);
        std::string requester = "Client " + std::to_string(user_id) + '\0';
        int num1 = std::rand() % NUM_MAX;
        int num2 = std::rand() % NUM_MAX;

        req.SetRequest(i, user_id, requester, num1, num2);
        std::cout << "[Sending ] ";
        req.PrintRequest();

        // TODO: complete this function in part_3_your_task.cpp
        // which marshalls req, sends req to the server, receives res from
        // the server, and unmarshalls res.
        part_3_client_marshal_send_recv_unmarshal(req, res, buffer, sockfd);

        std::cout << "[Received] ";
        res.PrintResponse();
    }


	close(sockfd);
	return 1;
}


