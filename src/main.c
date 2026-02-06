#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>

#include "headers/globs.h"
#include "headers/sender.h"
#include "headers/config.h"
#include "headers/main.h"

int main(int argc, char *argv[]){
    char* config;
    printf("Starting lct...\n");
    if (argc < 2){
        config = "config.ini";
    } else {
        config = argv[1];
    }
    parse_config(config);
    print_config();
    printf("Setting globs...\n");
    set_globs();

    if (IS_SERVER){
        printf("Starting in server mode...\n");
    } else {
        printf("Starting in client mode...\n");
        bool sock_alive = 0;
        int sleep_current = 0;
        while(sock_alive == 0){
            sock_alive = sock_init();
            if (!sock_alive){
                if (sleep_current < MAX_SOCK_SLEEP_SEC){
                    sleep_current += 3;
                }
                sleep(sleep_current);
            }
        }
    }
}
