#include <limits.h>
#include <pthread.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>

#include "headers/globs.h"
#include "headers/config.h"
#include "headers/main.h"
#include "headers/lct_socket.h"
#include "headers/utils.h"
#include "headers/comms.h"
#include "headers/udp_socket.h"

struct lct_packet_buffer* LCT_LISTEN_BUFFER;
struct udp_buffer* UDP_LISTEN_BUFFER;
pthread_mutex_t lct_buf_lock;
pthread_cond_t lct_comms_signal;
pthread_mutex_t udp_buf_lock;
pthread_cond_t udp_comms_signal;
int PEER_SOCKET_FD = -1;
int UDP_SOCKET_FD = -1;
bool LCT_CONN_ESTABLISHED = 0;

void lct_initiator_thread_guard(){
    printf("I'm the initiator. Initiating connection...\n");
    int lct_listener_thr_status;
    pthread_t lct_listener_thr_id = ULONG_MAX;
    while(1){
        if (sock_init()){
            break;
        }
        sleep(1);
    }
    // If a socket is dead or we know that connection is ruptured, kill the listener thread, try to re-initialize the socket and restart the listener thread. Do this indefinitely.
    while(1){
        if (!check_sock_alive(PEER_SOCKET_FD) || !LCT_CONN_ESTABLISHED){
            printf("Main channel ruptured. Reconnecting...\n");
            if(lct_listener_thr_id != ULONG_MAX){
                if(pthread_kill(lct_listener_thr_id, 0) == 0){
                    pthread_cancel(lct_listener_thr_id);
                    pthread_join(lct_listener_thr_id, NULL);
                }
                lct_listener_thr_id = ULONG_MAX;
            }
            sock_init();
        }
        if (LCT_CONN_ESTABLISHED && lct_listener_thr_id == ULONG_MAX){
            lct_listener_thr_status = pthread_create(&lct_listener_thr_id, NULL, lct_listen, NULL);
        }
        sleep(1);
    }
}

void lct_non_initiator_thread_guard(){
    printf("I'm not the initiator. Waiting to get initiated...\n");
    int lct_listener_thr_status;
    pthread_t lct_listener_thr_id = ULONG_MAX;
    lct_listener_init();
    lct_listener_thr_status = pthread_create(&lct_listener_thr_id, NULL, lct_listen, NULL);
    while(1){
        if (!check_sock_alive(PEER_SOCKET_FD)){
            if(lct_listener_thr_id != ULONG_MAX && LCT_CONN_ESTABLISHED){
                LCT_CONN_ESTABLISHED = 0;
                if(pthread_kill(lct_listener_thr_id, 0) == 0){ // sendind signal 0 returns 0 if a thread is alive
                    printf("Main channel ruptured. Restarting listener...\n");
                    pthread_cancel(lct_listener_thr_id);
                    pthread_join(lct_listener_thr_id, NULL);
                    lct_listener_thr_id = ULONG_MAX;
                    lct_listener_init();
                    lct_listener_thr_status = pthread_create(&lct_listener_thr_id, NULL, lct_listen, NULL);
                }
            }
        }
        sleep(1);
    }
}

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

    int lct_processor_thr_status;
    pthread_t lct_processor_thr_id;
    lct_processor_thr_status = pthread_create(&lct_processor_thr_id, NULL, lct_processor_loop, NULL);

    int udp_processor_thr_status;
    pthread_t udp_processor_thr_id;
    udp_processor_thr_status = pthread_create(&udp_processor_thr_id, NULL, udp_processor_loop, NULL);

    int udp_reciever_thr_status;
    pthread_t udp_reciever_thr_id;
    udp_reciever_thr_status = pthread_create(&udp_reciever_thr_id, NULL, udp_listen, NULL);

    if (IS_INITIATOR){
        lct_initiator_thread_guard();
    } else {
        lct_non_initiator_thread_guard();
    }
}

