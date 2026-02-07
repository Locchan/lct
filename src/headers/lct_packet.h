#pragma once

#include <stdint.h>

#include "globs.h"

#define PROTO_VERSION "0.1"

extern uint8_t valid_packet_types[2];

struct lct_packet{
    uint8_t type;
    char session_id[8];
    uint16_t data_size;
    uint16_t garbage_size;
    byte* data;
    byte* garbage;
};

byte* serialize_lct_packet(struct lct_packet* lctp, uint16_t* result_size);
struct lct_packet* deserialize_lct_packet(byte* lctp, uint16_t data_size);
void destroy_lct_packet(struct lct_packet* lctp);
uint16_t get_lct_packet_size(struct lct_packet* lctp);
struct lct_packet* create_lct_packet(char* session_id, byte* data, uint8_t type, uint16_t data_size);
void print_lct_packet(struct lct_packet* lctp);