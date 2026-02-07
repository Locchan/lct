#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#include "headers/sender.h"
#include "headers/globs.h"

int SOCKET_FD = 0;
bool SOCKET_ALIVE = 0;
struct sockaddr_in SERVER_ADDR;
bool sender_initialized = 0;

void sender_init(){
    memset(&SERVER_ADDR, 0, sizeof(SERVER_ADDR));
    SERVER_ADDR.sin_family = AF_INET;
    SERVER_ADDR.sin_port = htons(LCT_SERVER_PORT);

    if (inet_pton(AF_INET, LCT_SERVER_ADDR, &SERVER_ADDR.sin_addr) <= 0) {
        perror("inet_pton");
        close(SOCKET_FD);
        sender_initialized = 0;
    }
    sender_initialized = 1;
}

bool sock_init() {
    if (!sender_initialized){
        sender_init();
    }

    SOCKET_FD = socket(AF_INET, SOCK_STREAM, 0);
    if (SOCKET_FD < 0) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    if (connect(SOCKET_FD, (struct sockaddr *)&SERVER_ADDR, sizeof(struct sockaddr_in)) < 0) {
        perror("connect");
        close(SOCKET_FD);
        return 0;
    }

    printf("Socket initialized.\n");
    SOCKET_ALIVE = 1;
    return 1;
}

void send_data(byte* data, int size){
    size_t data_written = write(SOCKET_FD, data, size);
    if (data_written == -1){
        perror("Could not send data: ");
        SOCKET_ALIVE = 0;
        return;
    }
    if (data_written != size){
        fprintf(stderr, "Could not send all data. Only %ldb sent.\n", data_written);
    }
    printf("Sent %d bytes of data\n", size);
}