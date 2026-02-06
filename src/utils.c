#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include "headers/utils.h"

void crash(int exitcode, char* text, ...){
    va_list args;
    va_start(args, text);
    vfprintf(stderr, text, args);
    exit(exitcode);
}