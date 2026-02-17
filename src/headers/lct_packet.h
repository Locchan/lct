#pragma once

#include <stdint.h>

#include "globs.h"

#define PROTO_VERSION "0.1"

extern uint8_t LCT_PKT_VALID_TYPES[2];
extern byte LCT_PKT_MARKER[4];
extern uint8_t LCT_PKT_HEADER_SIZE;

struct lct_packet{
    byte marker[4];
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
// Returns true if a packet has been successfully extracted as well as the memcpy-ed part of the buffer with the packet and its size in this case.
bool extract_lct_packet(byte* buffer, uint16_t buffer_len, uint16_t lct_packet_position, byte** output_buffer, uint16_t* output_buffer_len);
// Returns the place in the buffer where the packet starts (if packet is found, MAX_UINT16T otherwise)
uint16_t detect_lct_packet(byte* buffer, uint16_t buffer_len);
uint16_t smart_garbage_sizer(uint16_t max_garbage_size, uint16_t data_size);