#include <netinet/in.h> //structure for storing address information
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h> //for socket APIs
#include <sys/types.h>
#include <string.h>
  
int main(int argc, char const* argv[])
{
    int sockD = socket(AF_INET, SOCK_STREAM, 0);
  
    struct sockaddr_in servAddr;
  
    servAddr.sin_family = AF_INET;
    servAddr.sin_port = htons(4866); // use some unused port number
    servAddr.sin_addr.s_addr = INADDR_ANY;
  
    int connectStatus
        = connect(sockD, (struct sockaddr*)&servAddr,
                  sizeof(servAddr));
  
    if (connectStatus == -1) {
        printf("Error...\n");
    } else {
		char buffer[255];
		const char* quit = "QUIT";
		char msg[] = "HELLO Server";

		while (1) {
			printf("Client: ");
			scanf("%[^\n]%*c", buffer);
			send(sockD, buffer, sizeof(buffer), 0);
			int r = recv(sockD, buffer, sizeof(buffer), 0);
			if (r > 0) printf("Server: %s\n", buffer);
			if (strcmp(buffer, quit) == 0) break;
		}
	}
  
    return 0;
}
