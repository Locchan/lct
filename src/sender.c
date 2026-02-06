#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#include "headers/sender.h"
#include "headers/globs.h"

int SOCKET_FD = 0;

bool sock_init() {
    struct sockaddr_in server_addr;

    SOCKET_FD = socket(AF_INET, SOCK_STREAM, 0);
    if (SOCKET_FD < 0) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(12345);

    if (inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr) <= 0) {
        perror("inet_pton");
        close(SOCKET_FD);
        return 0;
    }

    if (connect(SOCKET_FD, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("connect");
        close(SOCKET_FD);
        return 0;
    }

    printf("Socket initialized.\n");
    return 1;
}
