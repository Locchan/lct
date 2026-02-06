#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "headers/config.h"
#include "headers/globs.h"
#include "headers/utils.h"

struct config_entry* GLOBAL_CONFIG = NULL;
bool IS_SERVER = false;
char* LCT_LISTEN_ADDR = NULL;
int LCT_LISTEN_PORT = 0;
char* LCT_SERVER_ADDR = NULL;
int LCT_SERVER_PORT = 0;

void parse_config(char *config_path){
    printf("Parsing config...\n");
    FILE *configfile = fopen(config_path, "r");

    if (configfile == NULL) {
        char* buffer = malloc(50 * sizeof(char));
        snprintf(buffer, 50, "Error opening file '%s'", config_path);
        perror(buffer);
        free(buffer);
        exit(EXIT_FAILURE);
    }

    GLOBAL_CONFIG = (struct config_entry*) malloc(sizeof(struct config_entry));
    GLOBAL_CONFIG->first = GLOBAL_CONFIG;
    GLOBAL_CONFIG->next = NULL;
    GLOBAL_CONFIG->key = NULL;
    GLOBAL_CONFIG->value = NULL;

    struct config_entry *current_config = GLOBAL_CONFIG;

    const char *separator = "=";
    char *line = malloc(MAX_LINESIZE);
    while (fgets(line, MAX_LINESIZE, configfile)) {
        if(current_config->key != NULL){ // if key is null then we're at the first empty entry, no need to create a new one
            current_config->next = (struct config_entry*) malloc(sizeof(struct config_entry)); // initialize the new config object
            current_config = current_config->next; // make the new object current one
            current_config->first = GLOBAL_CONFIG; // first is always this
        }
        char *token = strtok(line, separator); // getting the key
        if (token != NULL){
            current_config->key = malloc((strlen(token) + 1) * sizeof(char));
            strcpy(current_config->key, token);
            token = strtok(NULL, separator); // getting the value
            if (token != NULL){
                current_config->value = malloc((strlen(token) + 1) * sizeof(char));
                strcpy(current_config->value, token);
                size_t len = strlen(current_config->value);
                if (len > 0 && current_config->value[len - 1] == '\n') { // removing trailing newline if exists
                    current_config->value[len - 1] = '\0';
                }
            } else {
                crash(EXIT_FAILURE, "Malformed configuration. Config format is KEY=VAL; one pair per line; 255 chars per line at most.\n");
            }
        } else {
            crash(EXIT_FAILURE, "Malformed configuration. Config format is KEY=VAL; one pair per line; 255 chars per line at most.\n");
        }
    }
    free(line);
    fclose(configfile);
}

void print_config(){
    struct config_entry *current_config = GLOBAL_CONFIG;
    printf("Parsed config:\n");
    while(current_config != NULL){
        printf("\t%s -> %s\n", current_config->key, current_config->value);
        current_config = current_config->next;
    }
}

char* get_config_entry(char* key, bool crash_if_not_found){
    struct config_entry *current_config = GLOBAL_CONFIG;
    while(current_config != NULL){
        if (strcmp(key, current_config->key) == 0){
            return current_config->value;
        } else {
            current_config = current_config->next;
        }
    }
    if (crash_if_not_found){
        crash(EXIT_FAILURE, "Config entry '%s' is mandatory.\n", key);
    }
    return NULL;
}

void set_globs(){
    IS_SERVER = strcmp(get_config_entry("MODE", 1), "server") == 0;
    if (IS_SERVER){
        LCT_LISTEN_ADDR = get_config_entry("LCT_LISTEN_ADDR", 1);
        LCT_LISTEN_PORT = atoi(get_config_entry("LCT_LISTEN_PORT", 1));
        if (LCT_LISTEN_PORT <= 0 || LCT_LISTEN_PORT > 65535){
            crash(EXIT_FAILURE, "Malformed LCT_LISTEN_PORT. Should be a valid port number.\n");
        }
    } else {
        LCT_SERVER_ADDR = get_config_entry("LCT_SERVER_ADDR", 1);
        LCT_SERVER_PORT = atoi(get_config_entry("LCT_SERVER_PORT", 1));
        if (LCT_SERVER_PORT <= 0 || LCT_SERVER_PORT > 65535){
            crash(EXIT_FAILURE, "Malformed LCT_LISTEN_PORT. Should be a valid port number.\n");
        }
    }
}