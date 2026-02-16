#pragma once

#include <pthread.h>

#include "lct_packet.h"
#include "comms.h"
#include "globs.h"

struct udp_buffer {
    byte** data;
    uint16_t data_size;
    uint16_t size_items;
    uint16_t max_size_items;
    uint16_t head;
    uint16_t tail;
};

struct udp_pop_result {
    byte* data;
    uint16_t size;
};

extern pthread_mutex_t udp_buf_lock;

struct udp_buffer* udp_buf_initialize(uint16_t max_size_items);
void udp_buf_add(byte* data, uint16_t data_size, struct udp_buffer* buffer);
struct udp_pop_result udp_buf_pop(struct udp_buffer* buffer);