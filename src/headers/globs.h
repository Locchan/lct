#pragma once
#include <stdbool.h>
#include <stdint.h>

#define MAX_LCT_PACKET_SIZE 1400

typedef uint8_t byte;
extern bool IS_SERVER;
extern char* LCT_LISTEN_ADDR;
extern uint16_t LCT_LISTEN_PORT;
extern char* LCT_SERVER_ADDR;
extern uint16_t  LCT_SERVER_PORT;
extern char* ENCRYPTION_KEY;
extern char* SESSION_ID;

extern int SOCKET_FD;