#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <string.h>
#include <arpa/inet.h>
#include <malloc.h>
#include <string.h>

int main() {
    int client = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    struct addrinfo *info;
    int ret = getaddrinfo("www.axmag.com", "http", NULL, &info);

    if (ret == -1 || info == NULL) {
        printf("Failed to get IP address\n");
        return 1;
    }

    if (connect(client, info->ai_addr, info->ai_addrlen)) {
        perror("connect() failed");
        return 1;
    }

    // Gui REQ den server
    char *msg = "GET /download/pdfurl-guide.pdf HTTP/1.1\r\nHost: www.axmag.com\r\nConnection: close\r\n\r\n";
    send(client, msg, strlen(msg), 0);

    // Download header
    char *res = NULL;
    char buf[256];
    uint size = 0;
    char *pos = NULL;

    while (1) {
        int len = recv(client, buf, sizeof(buf), 0);
        if (len <= 0) {
            printf("Failed to download file.\n");
            close(client);
            exit(1);
        }

        res = (char *)realloc(res, size + len + 1);
        memcpy(res + size, buf, len);
        size += len;
        res[size] = '\0';

        if ((pos = strstr(res, "\r\n\r\n")) != NULL) {
            break;
        }
    }

    
    // Show header
    *pos = '\0';
    printf("Header:\n%s\n", res);


    // Read the remaining data and save to file
    FILE* f = fopen("def.pdf", "ab");
    
    fwrite(pos + 4, 1, size - (pos - res) - 3, f);

    while (1) {
        int len = recv(client, buf, sizeof(buf), 0);
        if (len <= 0) {
            break;
        }

        fwrite(buf, 1, len, f);
        size += len;
    }

    fclose(f);

    // Ket thuc, dong socket
    close(client);

    printf("Download %d bytes completed.\n", size);

    return 0;
}