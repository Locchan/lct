#pragma once
#include <stdbool.h>
#include <stdint.h>

#define MAX_LCT_PACKET_SIZE 2048

typedef uint8_t byte;
extern bool IS_INITIATOR;
extern char* LCT_PEER_ADDR;
extern uint16_t LCT_PEER_PORT;
extern char* UDP_LISTEN_ADDR;
extern uint16_t UDP_LISTEN_PORT;
extern char* UDP_SEND_ADDR;
extern uint16_t UDP_SEND_PORT;
extern char* ENCRYPTION_KEY;
extern char* SESSION_ID;

extern int PEER_SOCKET_FD;
extern int UDP_SOCKET_FD;
extern bool LCT_CONN_ESTABLISHED;