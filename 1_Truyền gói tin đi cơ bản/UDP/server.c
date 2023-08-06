#include <netinet/in.h> //structure for storing address information
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h> //for socket APIs
#include <sys/types.h>
#include <pthread.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <ctype.h>
#define BUFFER_SIZE 1024


int main() {
	// Create socket
	int sock = socket(AF_INET, SOCK_DGRAM, 0);
	printf("Initialized socket!\n");

	// Server address
	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(4889);
	addr.sin_addr.s_addr = INADDR_ANY;
	
	// Bind socket to address
	bind(sock, (struct sockaddr*) &addr, sizeof(addr));
	printf("Socket've been binded\n");

	int loop_flag = 1;
	char buffer[BUFFER_SIZE];
	const char* quit_flag = "QUIT";
	printf("New Client INIT\n");
	socklen_t slen = sizeof(struct sockaddr_in);

	while (loop_flag) {
        int r = recvfrom(sock, buffer, sizeof(buffer), 0, (struct sockaddr*) &addr, &slen);
		if (r <= 0) break;
		if (strcmp(buffer, quit_flag) == 0) loop_flag = 0;
		sendto(sock, buffer, BUFFER_SIZE, 0, (struct sockaddr*) &addr, slen);
	}	
	return 0;
}


