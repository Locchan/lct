#pragma once
#define MAX_LINESIZE 256 * sizeof(char)
struct config_entry{
    char* key;
    char* value;
    struct config_entry *next;
    struct config_entry *first;
};

extern struct config_entry* GLOBAL_CONFIG;
void parse_config(char *config_path);
void print_config();
void set_globs();