#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

#include "headers/lct_packet.h"
#include "headers/utils.h"
#include "headers/comms.h"
#include "headers/lct_buffer.h"
#include "headers/lct_socket.h"
#include "headers/udp_buffer.h"
#include "headers/udp_socket.h"

char* CURRENT_SESSION_ID = NULL;

void mutexes_init(){
    pthread_mutex_init(&lct_buf_lock, NULL);
    pthread_cond_init(&lct_comms_signal, NULL);
}

void process_lct_packet(struct lct_packet* packet){
    udp_send(packet->data, packet->data_size);
    destroy_lct_packet(packet);
}

void process_udp_datagram(struct udp_pop_result udp_data){
    struct lct_packet* packet;
    uint16_t* packet_size;
    byte* packet_serialized;
    if (LCT_CONN_ESTABLISHED){
        if (CURRENT_SESSION_ID == NULL){
            CURRENT_SESSION_ID = malloc(8 * sizeof(char));
            generate_session_id(CURRENT_SESSION_ID);
        }
        packet = create_lct_packet(CURRENT_SESSION_ID, udp_data.data, 1, udp_data.size);
        packet_serialized = serialize_lct_packet(packet, packet_size);
        send_data(packet_serialized, *packet_size);
        destroy_lct_packet(packet);
        free(packet_serialized);
    }
    free(udp_data.data);
}

void* lct_processor_loop(void* args){
    printf("LCT processor thread started.\n");
    struct lct_packet* packet = NULL;
    pthread_mutex_lock(&lct_buf_lock); 
    while(1){
        while (LCT_LISTEN_BUFFER->size_items == 0) {
            pthread_cond_wait(&lct_comms_signal, &lct_buf_lock); // this releases the mutex until there's a signal, then it will lock it automatically;
        }
        packet = lct_buf_pop(LCT_LISTEN_BUFFER);
        pthread_mutex_unlock(&lct_buf_lock);
        process_lct_packet(packet);
        pthread_mutex_lock(&lct_buf_lock);
    }
}

void* udp_processor_loop(void* args){
    printf("UDP processor thread started.\n");
    struct udp_pop_result udp_data;
    uint16_t data_size;
    pthread_mutex_lock(&udp_buf_lock); 
    while(1){
        while (UDP_LISTEN_BUFFER->size_items == 0) {
            pthread_cond_wait(&udp_comms_signal, &udp_buf_lock);
        }
        udp_data = udp_buf_pop(UDP_LISTEN_BUFFER);
        pthread_mutex_unlock(&udp_buf_lock);
        process_udp_datagram(udp_data);
        pthread_mutex_lock(&udp_buf_lock);
    }
}