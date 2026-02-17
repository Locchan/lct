#include <arpa/inet.h>
#include <asm-generic/socket.h>
#include <assert.h>
#include <stdint.h>
#include <stdio.h> 
#include <netdb.h> 
#include <netinet/in.h> 
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h> 
#include <sys/types.h> 
#include <unistd.h>

#include "headers/lct_socket.h"
#include "headers/globs.h"
#include "headers/lct_packet.h"
#include "headers/lct_buffer.h"

long long unsigned int total_transferred_lct = 0;

void lct_listener_init(){
    int server_fd;
    struct sockaddr_in addr = { .sin_family = AF_INET, .sin_port = htons(LCT_PEER_PORT) };
    if (inet_pton(AF_INET, LCT_PEER_ADDR, &addr.sin_addr) <= 0) {
        perror("Invalid IP address");
        return;
    }

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    
    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt)) < 0) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    bind(server_fd, (struct sockaddr *)&addr, sizeof(addr));
    listen(server_fd, 3);

    printf("LCT listening on %s:%d.\n", LCT_PEER_ADDR, LCT_PEER_PORT);
    PEER_SOCKET_FD = accept(server_fd, NULL, NULL);
    LCT_CONN_ESTABLISHED = 1;
    printf("LCT Connection established.\n");
}

void* lct_listen(void* args){
    printf("LCT listener thread started.\n");
    byte* lct_packet_serialized = NULL;
    uint16_t lct_packet_serialized_size = 0;
    uint16_t lct_packet_start_byte;
    bool pkt_extraction_success = 0;
    struct lct_packet* packet = NULL;
    byte read_buffer[MAX_LCT_PACKET_SIZE] = {0};
    byte data_buffer[3*MAX_LCT_PACKET_SIZE] = {0};
    uint16_t data_buf_size = 0;
    uint16_t data_buf_cutoff_start = 0;
    uint16_t size_recieved = 0;

    while(1) {
        for(int i=0; i<MAX_LCT_PACKET_SIZE; i++) read_buffer[i] = 0;
        size_recieved = recv(PEER_SOCKET_FD, read_buffer, sizeof(read_buffer) - 1, 0);
        if (size_recieved > 0){
            assert(data_buf_size + size_recieved < 3*MAX_LCT_PACKET_SIZE);
            memcpy(&data_buffer[data_buf_size], read_buffer, size_recieved);
            data_buf_size += size_recieved;
            do { // Iterate over the buffer and find all valid packets in it
                if (data_buf_size < LCT_PKT_HEADER_SIZE){
                    break;
                }
                lct_packet_start_byte = detect_lct_packet(data_buffer, data_buf_size);

                if(lct_packet_start_byte != UINT16_MAX){ // a packet header is found (detect_lct_packet returns UINT16_MAX if it is not so)
                    if (data_buf_size - lct_packet_start_byte > LCT_PKT_HEADER_SIZE + 1){ // if the packet can be valid at all (or at least the header is full)
                        pkt_extraction_success = extract_lct_packet(data_buffer, data_buf_size, lct_packet_start_byte, &lct_packet_serialized, &lct_packet_serialized_size);
                        if (pkt_extraction_success){
                            packet = deserialize_lct_packet(lct_packet_serialized, lct_packet_serialized_size);
                            if (packet == NULL) { // got something that has the valid marker and valid size, but is invalid otherwise. In this case, just remove sizeof(LCT_PKT_MARKER) bytes from the start (the false packet header)
                                data_buf_cutoff_start = lct_packet_start_byte + sizeof(LCT_PKT_MARKER);
                                memmove(data_buffer, &data_buffer[data_buf_cutoff_start], data_buf_size - (data_buf_cutoff_start));
                                data_buf_size -= lct_packet_start_byte + sizeof(LCT_PKT_MARKER);
                                continue;
                            }
                            /*double overhead = ((double)lct_packet_serialized_size - packet->data_size) / packet->data_size * 100.0;
                            printf("-> LCT: %db (%.2f overhead)\n", lct_packet_serialized_size, overhead);*/
                            free(lct_packet_serialized);
                            data_buf_cutoff_start = lct_packet_start_byte + lct_packet_serialized_size;
                            memmove(data_buffer, &data_buffer[data_buf_cutoff_start], data_buf_size - (data_buf_cutoff_start));
                            data_buf_size -= lct_packet_start_byte + lct_packet_serialized_size;
                            pthread_mutex_lock(&lct_buf_lock);
                            lct_buf_add(packet, LCT_LISTEN_BUFFER);
                            pthread_mutex_unlock(&lct_buf_lock);
                        } else {
                            if(data_buf_size - lct_packet_start_byte >= MAX_LCT_PACKET_SIZE){ // header is found but there's garbage data, not a valid packet (there is > MAX_LCT_PACKET_SIZE data in the buffer after the header but no valid packet was parsed)
                                data_buf_cutoff_start = lct_packet_start_byte + MAX_LCT_PACKET_SIZE;
                                memmove(data_buffer, &data_buffer[data_buf_cutoff_start], data_buf_size - (data_buf_cutoff_start));
                                data_buf_size -= lct_packet_start_byte + MAX_LCT_PACKET_SIZE;
                            }
                        }
                    } else {
                        break;
                    }
                } else { // a packet header is not found. Cut everything but the last sizeof(LCT_PKT_MARKER) bytes (in case we had partial/cutoff marker at the very end of the buffer)
                    data_buf_cutoff_start = data_buf_size - sizeof(LCT_PKT_MARKER);
                    memmove(data_buffer, &data_buffer[data_buf_cutoff_start], sizeof(LCT_PKT_MARKER));
                    data_buf_size = 3;
                }
            } while(pkt_extraction_success); 
        }
    }

    close(PEER_SOCKET_FD);
}

struct sockaddr_in SERVER_ADDR;
bool lct_sender_initialized = 0;

void lct_sender_init(){
    memset(&SERVER_ADDR, 0, sizeof(SERVER_ADDR));
    SERVER_ADDR.sin_family = AF_INET;
    SERVER_ADDR.sin_port = htons(LCT_PEER_PORT);

    if (inet_pton(AF_INET, LCT_PEER_ADDR, &SERVER_ADDR.sin_addr) <= 0) {
        perror("inet_pton");
        close(PEER_SOCKET_FD);
        lct_sender_initialized = 0;
    }
    lct_sender_initialized = 1;
}

bool sock_init() {
    if (!lct_sender_initialized){
        lct_sender_init();
    }

    PEER_SOCKET_FD = socket(AF_INET, SOCK_STREAM, 0);
    if (PEER_SOCKET_FD < 0) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    printf("Initiating LCT connection to %s:%d...\n", LCT_PEER_ADDR, LCT_PEER_PORT);
    if (connect(PEER_SOCKET_FD, (struct sockaddr *)&SERVER_ADDR, sizeof(struct sockaddr_in)) < 0) {
        perror("connect");
        close(PEER_SOCKET_FD);
        return 0;
    }

    printf("LCT Connection established.\n");
    LCT_CONN_ESTABLISHED = 1;
    return LCT_CONN_ESTABLISHED;
}

void send_data(byte* data, uint16_t size){
    total_transferred_lct += size;
    size_t data_written = write(PEER_SOCKET_FD, data, size);
    if (data_written == -1){
        perror("Could not send data: ");
        LCT_CONN_ESTABLISHED = 0;
        return;
    }
    if (data_written != size){
        fprintf(stderr, "Could not send all data. Only %ldb sent.\n", data_written);
    }
    // printf("<- LCT: %db (%lld)\n", size, total_transferred_lct);
}