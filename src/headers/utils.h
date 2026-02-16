#pragma once

#include "globs.h"

void crash(int exitcode, char* text, ...);
byte* generate_random(byte* buffer, uint16_t size);
char* generate_session_id(char* session_id);
void print_binary(byte* data, uint16_t data_size);