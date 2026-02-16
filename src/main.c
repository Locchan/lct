#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>

#include "headers/globs.h"
#include "headers/config.h"
#include "headers/main.h"
#include "headers/lct_listener.h"
#include "headers/comms.h"
#include "headers/udp_listener.h"

struct lct_packet_buffer* LCT_LISTEN_BUFFER;
struct udp_buffer* UDP_LISTEN_BUFFER;
pthread_mutex_t lct_buf_lock;
pthread_cond_t lct_comms_signal;
pthread_mutex_t udp_buf_lock;
pthread_cond_t udp_comms_signal;

void initialize(){
    LCT_LISTEN_BUFFER = lct_buf_initialize(LCT_BUFFER_MAX_SIZE_ITEMS);
    UDP_LISTEN_BUFFER = udp_buf_initialize(UDP_BUFFER_MAX_SIZE_ITEMS);
    pthread_mutex_init(&lct_buf_lock, NULL);
    pthread_cond_init(&lct_comms_signal, NULL);
    pthread_mutex_init(&udp_buf_lock, NULL);
    pthread_cond_init(&udp_comms_signal, NULL);
}

int main(int argc, char *argv[]){
    initialize();
    srand(time(NULL)); // Seed the random with current time

    char* config;
    printf("Starting lct...\n");
    if (argc < 2){
        config = "config.ini";
    } else {
        config = argv[1];
    }
    parse_config(config);
    print_config();
    printf("Setting globs...\n");
    set_globs();

    if (IS_SERVER){
        printf("Starting in server mode...\n");

        // comms thread
        int status;
        pthread_t threadID;
        status = pthread_create(&threadID, NULL, srv_comms_loop, NULL);
        if (status != 0) {
            printf("Error creating thread\n");
            exit(-1);
        }
        lct_listen();
    } else {
        printf("Starting in client mode...\n");
        int status;
        pthread_t threadID;
        status = pthread_create(&threadID, NULL, client_comms_loop, NULL);
        if (status != 0) {
            printf("Error creating thread\n");
            exit(-1);
        }
        udp_listen();
    }
}
