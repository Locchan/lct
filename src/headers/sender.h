#pragma once

#include <stdbool.h>

#include "globs.h"

extern struct sockaddr_in SERVER_ADDR;
extern bool SOCKET_ALIVE;
void sender_init();
bool sock_init();
void send_data(byte* data, int size);
