#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#include "headers/udp_socket.h"
#include "headers/udp_buffer.h"
#include "headers/globs.h"

long long unsigned int total_transferred_udp = 0;
byte* datagram_data;
struct sockaddr_in server_addr, client_addr, send_addr;

void* udp_listen(void* args){
    char buffer[MAX_DATAGRAM_SIZE];
    socklen_t addr_len = sizeof(client_addr);

    if ((UDP_SOCKET_FD = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(UDP_LISTEN_ADDR);
    server_addr.sin_port = htons(UDP_LISTEN_PORT);

    memset(&send_addr, 0, sizeof(send_addr));
    send_addr.sin_family = AF_INET;
    send_addr.sin_addr.s_addr = inet_addr(UDP_SEND_ADDR);
    send_addr.sin_port = htons(UDP_SEND_PORT);

    if (bind(UDP_SOCKET_FD, (const struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("UDP bind failed");
        close(UDP_SOCKET_FD);
        exit(EXIT_FAILURE);
    }

    printf("UDP listening on port %d.\n", UDP_LISTEN_PORT);
    printf("UDP will send to port %d.\n", UDP_SEND_PORT);

    while (1) {
        ssize_t recieved_size = recvfrom(UDP_SOCKET_FD, buffer, MAX_DATAGRAM_SIZE - 1, 0, (struct sockaddr *)&client_addr, &addr_len);
        datagram_data = malloc(recieved_size * sizeof(byte));
        // printf("-> UDP: %ldb\n", recieved_size);
        memcpy(datagram_data, buffer, recieved_size);
        if (recieved_size < 0) {
            perror("recvfrom failed");
            continue;
        }
        pthread_mutex_lock(&udp_buf_lock);
        udp_buf_add(datagram_data, recieved_size, UDP_LISTEN_BUFFER);
        pthread_mutex_unlock(&udp_buf_lock);
       
    }

    close(UDP_SOCKET_FD);
}

void udp_send(byte* data, uint16_t size){
    total_transferred_udp += size;
    //printf("<- UDP: %db (%lld)\n", size, total_transferred_udp);
    sendto(UDP_SOCKET_FD, data, size, 0,(const struct sockaddr *)&send_addr, sizeof(send_addr));
}