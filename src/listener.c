#include <arpa/inet.h>
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

#include "headers/listener.h"
#include "headers/globs.h"
#include "headers/lct_packet.h"
#include "headers/lct_buffer.h"

struct lct_packet_buffer* LISTEN_BUFFER;
uint32_t INP_BUFFER_MAX_SIZE_ITEMS = 512;

void lct_listen(){
    int server_fd, client_fd;
    struct sockaddr_in addr = { .sin_family = AF_INET, .sin_port = htons(LCT_LISTEN_PORT) };
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
    LISTEN_BUFFER = lct_buf_initialize(INP_BUFFER_MAX_SIZE_ITEMS);

    if (inet_pton(AF_INET, LCT_LISTEN_ADDR, &addr.sin_addr) <= 0) {
        perror("Invalid IP address");
        return;
    }

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    bind(server_fd, (struct sockaddr *)&addr, sizeof(addr));
    listen(server_fd, 3);

    printf("Listening on %s:%d...\n", LCT_LISTEN_ADDR, LCT_LISTEN_PORT);
    client_fd = accept(server_fd, NULL, NULL);

    while(1) {
        for(int i=0; i<MAX_LCT_PACKET_SIZE; i++) read_buffer[i] = 0;
        size_recieved = recv(client_fd, read_buffer, sizeof(read_buffer) - 1, 0);
        if (size_recieved > 0){
            assert(data_buf_size + size_recieved < 3*MAX_LCT_PACKET_SIZE);
            memcpy(&data_buffer[data_buf_size], read_buffer, size_recieved);
            data_buf_size += size_recieved;
            do { // Iterate over the buffer and find all valid packets in it
                if (data_buf_size < LCT_PKT_HEADER_SIZE){
                    break;
                }
                lct_packet_start_byte = detect_lct_packet(data_buffer, data_buf_size);

                if(lct_packet_start_byte != UINT16_MAX){ // a packet header is found
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
                            free(lct_packet_serialized);
                            data_buf_cutoff_start = lct_packet_start_byte + lct_packet_serialized_size;
                            memmove(data_buffer, &data_buffer[data_buf_cutoff_start], data_buf_size - (data_buf_cutoff_start));
                            data_buf_size -= lct_packet_start_byte + lct_packet_serialized_size;
                            lct_buf_add(packet, LISTEN_BUFFER);
                        } else {
                            if(data_buf_size - lct_packet_start_byte >= MAX_LCT_PACKET_SIZE){ // header is found but there's garbage data, not a valid packet (there is > MAX_LCT_PACKET_SIZE data in the buffer after the header but no valid packet was parsed)
                                data_buf_cutoff_start = lct_packet_start_byte + MAX_LCT_PACKET_SIZE;
                                memmove(data_buffer, &data_buffer[data_buf_cutoff_start], data_buf_size - (data_buf_cutoff_start));
                                data_buf_size -= lct_packet_start_byte + MAX_LCT_PACKET_SIZE;
                            }
                        }
                    }
                } else { // a packet header is not found. Cut everything but the last sizeof(LCT_PKT_MARKER) bytes (in case we had partial/cutoff marker)
                    data_buf_cutoff_start = data_buf_size - sizeof(LCT_PKT_MARKER);
                    memmove(data_buffer, &data_buffer[data_buf_cutoff_start], sizeof(LCT_PKT_MARKER));
                    data_buf_size = 3;
                }
            } while(pkt_extraction_success); 
        }
    }

    close(client_fd);
    close(server_fd);
}