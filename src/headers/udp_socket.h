#define MAX_DATAGRAM_SIZE 1500

#include "udp_buffer.h"

extern struct udp_buffer* UDP_LISTEN_BUFFER;

void* udp_listen(void* args);
void udp_send(byte* data, uint16_t size);