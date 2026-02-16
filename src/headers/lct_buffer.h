#pragma once

#include <pthread.h>

#include "lct_packet.h"
#include "comms.h"

struct lct_packet_buffer {
    struct lct_packet** packet;
    uint16_t size_items;
    uint16_t max_size_items;
    uint16_t head;
    uint16_t tail;
};

extern pthread_mutex_t lct_buf_lock;

struct lct_packet_buffer* lct_buf_initialize(uint16_t max_size_items);
void lct_buf_add(struct lct_packet* packet, struct lct_packet_buffer* buffer);
struct lct_packet* lct_buf_pop(struct lct_packet_buffer* buffer);