#pragma once
#include <stdbool.h>

extern struct sockaddr_in SERVER_ADDR;
extern bool SOCKET_ALIVE;
void sender_init();
bool sock_init();