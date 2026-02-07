#include <arpa/inet.h>
#include <stdio.h> 
#include <netdb.h> 
#include <netinet/in.h> 
#include <sys/socket.h> 
#include <sys/types.h> 
#include <unistd.h>

#include "headers/globs.h"
#include "headers/lct_packet.h"

void lct_listen(){
    int server_fd, client_fd;
    struct sockaddr_in addr = { .sin_family = AF_INET, .sin_port = htons(LCT_LISTEN_PORT) };
    struct lct_packet* packet;
    byte buffer[1400] = {0};

    if (inet_pton(AF_INET, LCT_LISTEN_ADDR, &addr.sin_addr) <= 0) {
        perror("Invalid IP address");
        return;
    }

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    bind(server_fd, (struct sockaddr *)&addr, sizeof(addr));
    listen(server_fd, 3);

    printf("Listening on %s:%d...\n", LCT_LISTEN_ADDR, LCT_LISTEN_PORT);
    client_fd = accept(server_fd, NULL, NULL);

    while (recv(client_fd, buffer, sizeof(buffer) - 1, 0) > 0) {
        packet = deserialize_lct_packet(buffer, 1400);
        if (packet == NULL){
            continue;
        }
        print_lct_packet(packet);
        destroy_lct_packet(packet);
        for(int i=0; i<1400; i++) buffer[i] = 0;
    }

    close(client_fd);
    close(server_fd);
}