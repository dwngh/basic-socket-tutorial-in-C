#include <netinet/in.h> //structure for storing address information
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h> //for socket APIs
#include <sys/types.h>
#include <string.h>
#define BUFFER_SIZE 128

  
int main(int argc, char const* argv[]) {
	int sock = socket(AF_INET, SOCK_STREAM, 0);

	struct sockaddr_in servAddr;

	servAddr.sin_family = AF_INET;
	servAddr.sin_port = htons(4889); // use some unused port number
	servAddr.sin_addr.s_addr = INADDR_ANY;

	int connectStatus= connect(sock, (struct sockaddr*)&servAddr, sizeof(servAddr));

	if (connectStatus == -1) {
		printf("Error...\n");
	} else {
		FILE* f = fopen("download.mp3", "wb");
		if (f == NULL) {
			printf("Open file error!\n");
			return;		
		}
	
		char buffer[BUFFER_SIZE];
		int size = 0;
		int len;
		while ((len = recv(sock, buffer, sizeof(buffer), 0)) > 0) {
			fwrite(buffer, 1, len, f);
			size += len;
		}
		printf("Received total %d bytes \n", size);
		fclose(f);
	}
}
		
