#include <stdio.h>
#include <stdlib.h>

#include "config.h"
#include "utils.h"

int main()
{
    struct config_t *main_conf = load_config("settings.txt");

    if(config_entry_exists(main_conf, "wasd2", "wasd") == 1)
    {
        printf("got entry exists wasd2.wasd\r\n");
        char *entry = config_entry_get(main_conf, "wasd2", "wasd");
        printf("got value of entry wasd2.wasd (%s)\r\n", entry);
        free(entry);
    }

    if(config_entry_exists(main_conf, NULL, "wasd") == 1)
    {
        printf("got entry exists wasd\r\n");
        char *entry = config_entry_get(main_conf, NULL, "wasd");
        printf("got value of entry wasd2.wasd (%s)\r\n", entry);
        free(entry);
    }

    if(config_entry_exists(main_conf, "wasd2.cap", "youisstupid") == 1)
    {
        printf("got entry exists wasd\r\n");
        char *entry = config_entry_get(main_conf, "wasd2.cap", "youisstupid");
        printf("got value of entry wasd2.cap (%s)\r\n", entry);
        free(entry);
    }

    if(config_entry_exists(main_conf, NULL, "wasddwasd") == 0)
    {
        printf("got fake entry not exists wasddwasd\r\n");
    }

    main_conf = config_entry_set(main_conf, NULL, "testing", "value");
    printf("set value testing: value\r\n");
    if(config_entry_exists(main_conf, NULL, "testing") == 1)
    {
        printf("got entry exists testing\r\n");
        char *entry = config_entry_get(main_conf, NULL, "testing");
        printf("got value of entry testing (%s)\r\n", entry);
        free(entry);
    }

    main_conf = config_entry_set(main_conf, "wasd2", "testing", "value");
    printf("set value wasd2.testing: value\r\n");
    if(config_entry_exists(main_conf, "wasd2", "testing") == 1)
    {
        printf("got entry exists wasd2.testing\r\n");
        char *entry = config_entry_get(main_conf, "wasd2", "testing");
        printf("got value of entry wasd2.testing (%s)\r\n", entry);
        free(entry);
    }


    free_config(&main_conf);
    
    return 0;
}