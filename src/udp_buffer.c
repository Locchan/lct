#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>

#include "headers/comms.h"
#include "headers/udp_buffer.h"

struct udp_buffer* udp_buf_initialize(uint16_t max_size_items){
    struct udp_buffer* new_buf = (struct udp_buffer*) malloc(sizeof(struct udp_buffer));
    new_buf->data = malloc(sizeof(byte*) * max_size_items);
    new_buf->data_size = malloc(sizeof(uint16_t) * max_size_items);
    new_buf->size_items = 0;
    new_buf->head = 0;
    new_buf->tail = 0;
    new_buf->max_size_items = max_size_items;
    return new_buf;
}

// THREAD UNSAFE, USE udp_buf_lock MUTEX
void udp_buf_add(byte* data, uint16_t data_size, struct udp_buffer* buffer){
    if(buffer->size_items == buffer->max_size_items){
        printf("UDP buffer is overflowing. We're losing data right now!\n");
        free(data);
        buffer->head = (buffer->head + 1) % buffer->max_size_items;
    } else {
        buffer->size_items += 1;
    }
    buffer->data[buffer->tail] = data;
    buffer->data_size[buffer->tail] = data_size;
    buffer->tail = (buffer->tail + 1) % buffer->max_size_items;
    pthread_cond_signal(&udp_comms_signal);
}

// THREAD UNSAFE, USE udp_buf_lock MUTEX
struct udp_pop_result udp_buf_pop(struct udp_buffer* buffer){
    struct udp_pop_result result;
    if(buffer->size_items > 0){
        result.data = buffer->data[buffer->head];
        result.size = buffer->data_size[buffer->head];
        buffer->head = (buffer->head + 1) % buffer->max_size_items;
        buffer->size_items -= 1;
    } else {
        result.data = NULL;
        result.size = 0;
    }
    return result;
}
