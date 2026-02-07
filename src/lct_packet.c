#include <assert.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>

#include "headers/globs.h"
#include "headers/lct_packet.h"
#include "headers/utils.h"

uint8_t valid_packet_types[2] = {1,2};
uint8_t header_size = sizeof(((struct lct_packet*)0)->type) + sizeof(((struct lct_packet*)0)->session_id) + sizeof(((struct lct_packet*)0)->data_size) + sizeof(((struct lct_packet*)0)->garbage_size);

struct lct_packet* create_lct_packet(char* session_id, byte* data, uint8_t type, uint16_t data_size){
    struct lct_packet* result = malloc(sizeof(struct lct_packet));

    result->type = type;

    for (uint8_t i=0; i<sizeof(result->session_id); i++){
        result->session_id[i] = session_id[i]; // dangerous, but I will make sure everywhere that session_id is strictly 8b
    }

    result->data = malloc(data_size);
    result->data_size = data_size;

    memcpy(result->data, data, data_size);

    uint16_t garbage_start = header_size + result->data_size;
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
    
    byte *current_ptr = data_serialized; // setting pointer to the first byte

    memcpy(current_ptr, &lctp->type, sizeof(lctp->type));
    current_ptr += sizeof(lctp->type); // then incrementing it every time we write anything

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

    if (data_size < header_size + 1){
        fprintf(stderr, "Got an invalid lct packet: impossibly small packet size: '%u'.", data_size);
        return NULL;
    }
    
    struct lct_packet* result = malloc(sizeof(struct lct_packet));
    
    byte *current_ptr = lctp;
    memcpy(&result->type, current_ptr, sizeof(result->type));
    current_ptr += sizeof(result->type);
    
    memcpy(result->session_id, current_ptr, sizeof(result->session_id));
    current_ptr += sizeof(result->session_id);

    memcpy(&result->data_size, current_ptr, sizeof(result->data_size));
    current_ptr += sizeof(result->data_size);

    memcpy(&result->garbage_size, current_ptr, sizeof(result->garbage_size));
    current_ptr += sizeof(result->garbage_size);

    uint16_t claimed_size = header_size + result->data_size + result->garbage_size;
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
    for(uint8_t i=0; i<sizeof(valid_packet_types)/sizeof(valid_packet_types[0]); i++){
        if (result->type == valid_packet_types[i]){
            finder = 1;
            break;
        }
    }

    if (finder == 0){
        fprintf(stderr, "Got an invalid lct packet: unknown packet type: '%u'.", result->type);
        free(result);
        return NULL;
    }

    uint16_t max_data_size = MAX_LCT_PACKET_SIZE - header_size;
    uint16_t max_garbage_size = MAX_LCT_PACKET_SIZE - (header_size + result->data_size);

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

void print_lct_packet(struct lct_packet* lctp){
    printf("LCT packet:\n");
    printf("\tType: %d\n", lctp->type);
    printf("\tSession ID: %.8s\n", lctp->session_id);
    printf("\tData length: %d\n", lctp->data_size);
    printf("\tGarbage length: %d\n", lctp->garbage_size);
    printf("\nData:\n");
    uint8_t counter = 0;
    printf("\t");
    for (uint16_t i=0; i<lctp->data_size; i++){
        if (counter == 8){
            printf("\n\t");
            counter = 0;
        }
        printf("%02x ", lctp->data[i]);
        counter+=1;
    }
    printf("\n");
}

void destroy_lct_packet(struct lct_packet* lctp){
    free(lctp->data);
    free(lctp->garbage);
    free(lctp);
}

uint16_t get_lct_packet_size(struct lct_packet* lctp){
    return header_size + lctp->data_size + lctp->garbage_size;
}