#pragma once

#include <pthread.h>

#include "lct_buffer.h"

extern char* CURRENT_SESSION_ID;

extern pthread_cond_t lct_comms_signal;
extern pthread_cond_t udp_comms_signal;

void* lct_processor_loop(void* args);
void* udp_processor_loop(void* args);