#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#include "headers/lct_packet.h"
#include "headers/globs.h"
#include "headers/sender.h"
#include "headers/config.h"
#include "headers/main.h"
#include "headers/utils.h"
#include "headers/listener.h"

int main(int argc, char *argv[]){
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
        lct_listen();
    } else {
        printf("Starting in client mode...\n");
        bool sock_alive = 0;
        int sleep_current = 0;
        while(sock_alive == 0){
            sock_alive = sock_init();
            if (!sock_alive){
                if (sleep_current < MAX_SOCK_SLEEP_SEC){
                    sleep_current += 3;
                }
                sleep(sleep_current);
            }
        }
        char session_id[8];
        generate_session_id(session_id);
        byte* data = "Hello, LCT!";
        struct lct_packet* packet;
        uint16_t* result_size;
        byte* packet_serialized;
        result_size = (uint16_t*) malloc(sizeof(uint16_t));
        while(1){
            packet = create_lct_packet(session_id, data, 1, 12);
            packet_serialized = serialize_lct_packet(packet, result_size);
            send_data(packet_serialized, *result_size);
            free(packet_serialized);
            usleep(50000);
            destroy_lct_packet(packet);
        }
    }
}
