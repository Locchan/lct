#pragma once
#include <stdbool.h>

extern bool IS_SERVER;
extern char* LCT_LISTEN_ADDR;
extern int LCT_LISTEN_PORT;
extern char* LCT_SERVER_ADDR;
extern int  LCT_SERVER_PORT;
extern char* ENCRYPTION_KEY;
extern char* SESSION_ID;

extern int SOCKET_FD;