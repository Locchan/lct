#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <sys/socket.h>
#include <errno.h>

#include "headers/globs.h"
#include "headers/utils.h"

void crash(int exitcode, char* text, ...){
    va_list args;
    va_start(args, text);
    vfprintf(stderr, text, args);
    exit(exitcode);
}

byte* generate_random(byte* buffer, uint16_t size){
    for (uint16_t i=0; i<size;i++){
        buffer[i] = rand() % 256;
    }
    return buffer;
}

char* generate_session_id(char* session_id){
    const char session_id_charset[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    for (uint16_t i=0; i<8;i++){
        session_id[i] = session_id_charset[rand() % 36];
    }
    return session_id;
}

void print_binary(byte* data, uint16_t data_size){
    printf("Data:\n");
    printf("\t");
    uint16_t counter = 0;
    for (uint16_t i=0; i<data_size; i++){
        if (counter == 8){
            printf("\n\t");
            counter = 0;
        }
        printf("%02x ", data[i]);
        counter+=1;
    }
    printf("\n");
}

int check_sock_alive(int sock_fd){
    if (sock_fd == -1){
        return 0;
    }
    char buf;
    // Try to "look" at 1 byte of data
    ssize_t result = recv(sock_fd, &buf, 1, MSG_PEEK | MSG_DONTWAIT);

    if (result == 0) {
        // Peer closed connection gracefully (received FIN)
        return 0; 
    } else if (result < 0) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            // No data available, but connection is still technically open
            return 1;
        }
        // A real error occurred (e.g., Connection Reset)
        return 0;
    }
    // result > 0: Data is waiting, connection is definitely alive
    return 1; 
}