#pragma once

#include "lct_packet.h"
#include "lct_buffer.h"

extern struct sockaddr_in SERVER_ADDR;
extern struct lct_packet_buffer* LCT_LISTEN_BUFFER;

void lct_sender_init();
bool sock_init();
void send_data(byte* data, uint16_t size);
void* lct_listen(void* args);
void lct_listener_init();
