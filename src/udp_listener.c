#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#include "headers/udp_listener.h"
#include "headers/udp_buffer.h"
#include "headers/globs.h"

byte* datagram_data;

void udp_listen(){
    int sockfd;
    char buffer[MAX_DATAGRAM_SIZE];
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_len = sizeof(client_addr);

    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(UDP_LISTEN_PORT);

    if (bind(sockfd, (const struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("UDP bind failed");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    printf("UDP listening on port %d...\n", UDP_LISTEN_PORT);

    while (1) {
        ssize_t recieved_size = recvfrom(sockfd, buffer, MAX_DATAGRAM_SIZE - 1, 0, (struct sockaddr *)&client_addr, &addr_len);
        datagram_data = malloc(recieved_size * sizeof(byte));
        memcpy(datagram_data, buffer, recieved_size);
        if (recieved_size < 0) {
            perror("recvfrom failed");
            continue;
        }
        pthread_mutex_lock(&udp_buf_lock);
        udp_buf_add(datagram_data, recieved_size, UDP_LISTEN_BUFFER);
        pthread_mutex_unlock(&udp_buf_lock);
       
    }

    close(sockfd);
}