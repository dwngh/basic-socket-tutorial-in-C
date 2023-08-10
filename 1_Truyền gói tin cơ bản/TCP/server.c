#include <netinet/in.h> //structure for storing address information
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h> //for socket APIs
#include <sys/types.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>


void* newClient(int client) {
	int loop_flag = 1;
	char buffer[255];
	const char* quit_flag = "QUIT";

	printf("New Client INIT\n");

	while (loop_flag) {
        int r = recv(client, buffer, sizeof(buffer), 0);
		if (r <= 0) break;
		if (strcmp(buffer, quit_flag) == 0) loop_flag = 0;
		send(client, buffer, sizeof(buffer), 0);
	}
	printf("A client closed connection!\n");
}

int main() {
	// Create socket
	int sock = socket(AF_INET, SOCK_STREAM, 0);
	printf("Initialized socket!\n");

	// Server address
	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(4866);
	addr.sin_addr.s_addr = INADDR_ANY;
	
	// Bind socket to address
	bind(sock, (struct sockaddr*) &addr, sizeof(addr));
	printf("Socket've been binded\n");

	// Start listening
	listen(sock, 1);
	printf("Start listening\n");
	int client;
	while (1) {	
		client =  accept(sock, NULL, NULL);

		newClient(client);
	}	
	return 0;
}

