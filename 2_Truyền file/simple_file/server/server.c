#include <netinet/in.h> //structure for storing address information
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h> //for socket APIs
#include <sys/types.h>
#include <pthread.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>
#define BUFFER_SIZE 256

int main() {
	// Create socket
	int sock = socket(AF_INET, SOCK_STREAM, 0);
	printf("Initialized socket!\n");

	// Server address
	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(4889);
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
		char buffer[BUFFER_SIZE];
		int b;
		
		FILE* file = fopen("download.mp3", "rb");
		if (file == NULL) {
			printf("Source file not found!\n");
			sprintf(buffer, "File not found");
			send(client, buffer, strlen(buffer) * sizeof(char), 0);	
		} else {
			int len = NULL;
			int total = 0;
			while ((len = fread(buffer, 1, BUFFER_SIZE, file)) > 0) {
				send(client, buffer, len, 0);
				total = total + len;
			}
			printf("Sent %d bytes\n", total);	
		}
		close(client);
	}	
	return 0;
}
