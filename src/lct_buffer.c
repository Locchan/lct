#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>

#include "headers/comms.h"
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

// THREAD UNSAFE, USE lct_buf_lock MUTEX
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
    pthread_cond_signal(&lct_comms_signal);
}

// THREAD UNSAFE, USE lct_buf_lock MUTEX
struct lct_packet* lct_buf_pop(struct lct_packet_buffer* buffer){
    struct lct_packet* packet;
    if(buffer->size_items > 0){
        packet = buffer->packet[buffer->head];
        buffer->head = (buffer->head + 1) % buffer->max_size_items;
        buffer->size_items -= 1;
        return packet;
    } else {
        return NULL;
    }
}