#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>

#include "headers/lct_buffer.h"
#include "headers/lct_packet.h"


struct lct_packet_buffer* lct_buf_initialize(uint16_t max_size_items){
    struct lct_packet_buffer* new_buf = (struct lct_packet_buffer*) malloc(sizeof(struct lct_packet_buffer));
    new_buf->packet = malloc(sizeof(struct lct_packet*) * max_size_items);
    new_buf->size_items = 0;
    new_buf->head = 0;
    new_buf->tail = 0;
    new_buf->max_size_items = max_size_items;
    return new_buf;
}

void lct_buf_add(struct lct_packet* packet, struct lct_packet_buffer* buffer){
    if(buffer->size_items == buffer->max_size_items){
        printf("LCT packet buffer is overflowing. We're losing data right now!\n");
        destroy_lct_packet(buffer->packet[buffer->tail]);
        buffer->head = (buffer->head + 1) % buffer->max_size_items;
    } else {
        buffer->size_items += 1;
    }
    buffer->packet[buffer->tail] = packet;
    buffer->tail = (buffer->tail + 1) % buffer->max_size_items;
}

struct lct_packet* lct_buf_pop(struct lct_packet_buffer* buffer){
    if(buffer->size_items > 0){
        struct lct_packet* packet = buffer->packet[buffer->head];
        buffer->head = (buffer->head + 1) % buffer->max_size_items;
        buffer->size_items -= 1;
        return packet;
    } else {
        return NULL;
    }
}