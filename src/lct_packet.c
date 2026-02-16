#include <assert.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>

#include "headers/globs.h"
#include "headers/lct_packet.h"
#include "headers/utils.h"

uint8_t LCT_PKT_VALID_TYPES[2] = {1,2};
byte LCT_PKT_MARKER[4] = {'L', 'C', 'T', '1'};
uint8_t LCT_PKT_HEADER_SIZE = 
    sizeof(((struct lct_packet*)0)->marker) +
    sizeof(((struct lct_packet*)0)->type) + 
    sizeof(((struct lct_packet*)0)->session_id) +
    sizeof(((struct lct_packet*)0)->data_size) +
    sizeof(((struct lct_packet*)0)->garbage_size);
uint64_t packet_number = 0;

struct lct_packet* create_lct_packet(char* session_id, byte* data, uint8_t type, uint16_t data_size){
    struct lct_packet* result = malloc(sizeof(struct lct_packet));

    result->type = type;

    for (uint8_t i=0; i<sizeof(result->marker); i++){
        result->marker[i] = LCT_PKT_MARKER[i];
    }

    for (uint8_t i=0; i<sizeof(result->session_id); i++){
        result->session_id[i] = session_id[i]; // dangerous, but I will make sure everywhere that session_id is strictly 8b
    }

    result->data = malloc(data_size);
    result->data_size = data_size;

    memcpy(result->data, data, data_size);

    uint16_t garbage_start = LCT_PKT_HEADER_SIZE + result->data_size;
    uint16_t max_garbage_size = MAX_LCT_PACKET_SIZE - garbage_start;
    if (max_garbage_size > 0) {
        uint16_t actual_garbage_size = rand() % max_garbage_size;
        byte* garbage = malloc(actual_garbage_size);
        generate_random(garbage, actual_garbage_size);
        result->garbage_size = actual_garbage_size;
        result->garbage = garbage;
    } else {
        result->garbage_size = 0;
        result->garbage = NULL;
    }

    return result;
}

byte* serialize_lct_packet(struct lct_packet* lctp, uint16_t* result_size){
    uint16_t lctp_size = get_lct_packet_size(lctp);
    *result_size = lctp_size;
    assert(lctp_size <= MAX_LCT_PACKET_SIZE); // Can't be more than that because I said so
    byte* data_serialized = (byte*) malloc(lctp_size);
    
    byte* current_ptr = data_serialized; // setting pointer to the first byte

    memcpy(current_ptr, &lctp->marker, sizeof(lctp->marker));
    current_ptr += sizeof(lctp->marker);  // then incrementing it every time we write anything

    memcpy(current_ptr, &lctp->type, sizeof(lctp->type));
    current_ptr += sizeof(lctp->type);

    memcpy(current_ptr, lctp->session_id, sizeof(lctp->session_id));
    current_ptr += sizeof(lctp->session_id);

    memcpy(current_ptr, &lctp->data_size, sizeof(lctp->data_size));
    current_ptr += sizeof(lctp->data_size);

    memcpy(current_ptr, &lctp->garbage_size, sizeof(lctp->garbage_size));
    current_ptr += sizeof(lctp->garbage_size);

    memcpy(current_ptr, lctp->data, lctp->data_size);
    current_ptr += lctp->data_size;

    if (lctp->garbage_size > 0){
        memcpy(current_ptr, lctp->garbage, lctp->garbage_size);
    }

    return data_serialized;
}

struct lct_packet* deserialize_lct_packet(byte* lctp, uint16_t data_size){

    if (data_size < LCT_PKT_HEADER_SIZE + 1){
        fprintf(stderr, "Got an invalid lct packet: impossibly small packet size: '%u'.", data_size);
        return NULL;
    }
    
    struct lct_packet* result = malloc(sizeof(struct lct_packet));
    
    byte *current_ptr = lctp;
    memcpy(&result->marker, current_ptr, sizeof(result->marker));
    current_ptr += sizeof(result->marker);

    memcpy(&result->type, current_ptr, sizeof(result->type));
    current_ptr += sizeof(result->type);
    
    memcpy(result->session_id, current_ptr, sizeof(result->session_id));
    current_ptr += sizeof(result->session_id);

    memcpy(&result->data_size, current_ptr, sizeof(result->data_size));
    current_ptr += sizeof(result->data_size);

    memcpy(&result->garbage_size, current_ptr, sizeof(result->garbage_size));
    current_ptr += sizeof(result->garbage_size);

    uint16_t claimed_size = LCT_PKT_HEADER_SIZE + result->data_size + result->garbage_size;
    if (claimed_size > MAX_LCT_PACKET_SIZE){
        fprintf(stderr, "Got an invalid lct packet: impossibly large claimed size: '%u'.", claimed_size);
        free(result);
        return NULL;
    }

    if (claimed_size > data_size){
        fprintf(stderr, "Got an invalid lct packet: claimed size bigger than total data recieved: '%u>%u'.", claimed_size, data_size);
        free(result);
        return NULL;
    }

    bool finder = 0;
    for(uint8_t i=0; i<sizeof(LCT_PKT_VALID_TYPES)/sizeof(LCT_PKT_VALID_TYPES[0]); i++){
        if (result->type == LCT_PKT_VALID_TYPES[i]){
            finder = 1;
            break;
        }
    }

    if (finder == 0){
        fprintf(stderr, "Got an invalid lct packet: unknown packet type: '%u'.", result->type);
        free(result);
        return NULL;
    }

    uint16_t max_data_size = MAX_LCT_PACKET_SIZE - LCT_PKT_HEADER_SIZE;
    uint16_t max_garbage_size = MAX_LCT_PACKET_SIZE - (LCT_PKT_HEADER_SIZE + result->data_size);

    if (result->data_size > max_data_size){
        fprintf(stderr, "Got an invalid lct packet: invalid data length: '%u'.", result->data_size);
        free(result);
        return NULL;
    }

    if (result->garbage_size > max_garbage_size){
        fprintf(stderr, "Got an invalid lct packet: invalid garbage length: '%u'.", result->garbage_size);
        free(result);
        return NULL;
    }

    result->data = malloc(result->data_size);
    memcpy(result->data, current_ptr, result->data_size);
    current_ptr += result->data_size;

    if (result->garbage_size > 0) {
        result->garbage = malloc(result->garbage_size);
        memcpy(result->garbage, current_ptr, result->garbage_size);
    } else {
        result-> garbage = NULL;
    }

    return result;
}

// This will return UINT16_MAX if a packet has not been found or packet start position in the buffer otherwise
uint16_t detect_lct_packet(byte* buffer, uint16_t buffer_len){
    for (uint16_t i=0; i<=buffer_len-sizeof(LCT_PKT_MARKER); i++){ // -sizeof(marker) is for the case when the marker is at the very end.
        if (memcmp(LCT_PKT_MARKER, &buffer[i], sizeof(LCT_PKT_MARKER)) == 0){
            return i;
        }
    }
    return UINT16_MAX;
}

bool extract_lct_packet(byte* buffer, uint16_t buffer_len, uint16_t lct_packet_position, byte** output_buffer, uint16_t* output_buffer_len){
    uint16_t data_size;
    uint16_t garbage_size;
    byte* pointer = &buffer[lct_packet_position];
    pointer += sizeof(((struct lct_packet*)0)->marker) +
                sizeof(((struct lct_packet*)0)->type) + 
                sizeof(((struct lct_packet*)0)->session_id);
    memcpy(&data_size, pointer, sizeof(uint16_t));
    pointer += sizeof(uint16_t);
    memcpy(&garbage_size, pointer, sizeof(uint16_t));
    if (buffer_len - lct_packet_position < LCT_PKT_HEADER_SIZE + data_size + garbage_size || // buffer has incomplete packet
        LCT_PKT_HEADER_SIZE + data_size + garbage_size > MAX_LCT_PACKET_SIZE){ // or garbage packet with more than allowed size
        return 0;
    }
    *output_buffer_len = LCT_PKT_HEADER_SIZE + data_size + garbage_size;
    *output_buffer = malloc(*output_buffer_len * sizeof(byte));
    memcpy(*output_buffer, &buffer[lct_packet_position], *output_buffer_len);
    return 1;
}

void print_lct_packet(struct lct_packet* lctp){
    packet_number += 1;
    printf("Recieved packet #%lu:\n", packet_number);
    printf("\tType: %d\n", lctp->type);
    printf("\tSession ID: %.8s\n", lctp->session_id);
    printf("\tData length: %d\n", lctp->data_size);
    printf("\tGarbage length: %d\n", lctp->garbage_size);
    printf("\nData:\n");
    print_binary(lctp->data, lctp->data_size);
    printf("\n");
}

void destroy_lct_packet(struct lct_packet* lctp){
    free(lctp->data);
    free(lctp->garbage);
    free(lctp);
    lctp = NULL;
}

uint16_t get_lct_packet_size(struct lct_packet* lctp){
    return LCT_PKT_HEADER_SIZE + lctp->data_size + lctp->garbage_size;
}