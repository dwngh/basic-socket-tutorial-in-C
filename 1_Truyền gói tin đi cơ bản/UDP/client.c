#include <netinet/in.h> //structure for storing address information
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h> //for socket APIs
#include <sys/types.h>
#include <string.h>
#define BUFFER_SIZE 1024

socklen_t slen = sizeof(struct sockaddr_in);

  
int main(int argc, char const* argv[]) {
	int sockD = socket(AF_INET, SOCK_DGRAM, 0);

	struct sockaddr_in servAddr;

	servAddr.sin_family = AF_INET;
	servAddr.sin_port = htons(4889); // use some unused port number
	servAddr.sin_addr.s_addr = INADDR_ANY;

	int connectStatus = 1;

	if (connectStatus == -1) {
		printf("Error...\n");
	} else {
		char buffer[BUFFER_SIZE];
		const char* quit = "QUIT";
		char msg[] = "HELLO Server";
		while (1) {
			printf("Client: ");
			scanf("%[^\n]%*c", buffer);
			sendto(sockD, buffer, sizeof(buffer), 0, (struct sockaddr*)&servAddr, slen);
			if (strcmp(buffer, quit) == 0) break;
			int r = recvfrom(sockD, buffer, sizeof(buffer), 0, (struct sockaddr*)&servAddr, &slen);
			if (r > 0) printf("Server: %s\n", buffer);
		}
		
	}

	return 0;
}
