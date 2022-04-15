#include <stdio.h>
#include <stdlib.h>

#include "config.h"
#include "configutils.h"

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
    
    printf("\r\n");

    if(config_entry_exists(main_conf, NULL, "wasd") == 1)
    {
        printf("got entry exists wasd\r\n");
        char *entry = config_entry_get(main_conf, NULL, "wasd");
        printf("got value of entry wasd2.wasd (%s)\r\n", entry);
        free(entry);
    }

    printf("\r\n");

    if(config_entry_exists(main_conf, "wasd2.cap", "youisstupid") == 1)
    {
        printf("got entry exists wasd\r\n");
        char *entry = config_entry_get(main_conf, "wasd2.cap", "youisstupid");
        printf("got value of entry wasd2.cap (%s)\r\n", entry);
        free(entry);
    }

    printf("\r\n");

    if(config_entry_exists(main_conf, NULL, "wasddwasd") == 0)
    {
        printf("got fake entry not exists wasddwasd\r\n");
    }

    printf("\r\n");

    main_conf = config_entry_set(main_conf, NULL, "testing", "value");
    printf("set value testing: value\r\n");
    if(config_entry_exists(main_conf, NULL, "testing") == 1)
    {
        printf("got entry exists testing\r\n");
        char *entry = config_entry_get(main_conf, NULL, "testing");
        printf("got value of entry testing (%s)\r\n", entry);
        free(entry);
    }

    printf("\r\n");

    main_conf = config_entry_set(main_conf, "wasd2", "testing", "value");
    printf("set value wasd2.testing: value\r\n");
    if(config_entry_exists(main_conf, "wasd2", "testing") == 1)
    {
        printf("got entry exists wasd2.testing\r\n");
        char *entry = config_entry_get(main_conf, "wasd2", "testing");
        printf("got value of entry wasd2.testing (%s)\r\n", entry);
        free(entry);
    }

    printf("\r\n");

    main_conf = config_entry_set(main_conf, "wasd2.cap", "testing", "value");
    printf("set value wasd2.cap.testing: value\r\n");
    if(config_entry_exists(main_conf, "wasd2.cap", "testing") == 1)
    {
        printf("got entry exists wasd2.cap.testing\r\n");
        char *entry = config_entry_get(main_conf, "wasd2.cap", "testing");
        printf("got value of entry wasd2.cap.testing (%s)\r\n", entry);
        free(entry);
    }

    printf("\r\n");

    main_conf = config_entry_set(main_conf, NULL, "wasd", NULL);
    printf("removed entry wasd\r\n");
    if(config_entry_exists(main_conf, NULL, "wasd") == 1)
    {
        printf("got entry exists wasd\r\n");
        char *entry = config_entry_get(main_conf, NULL, "wasd");
        printf("got value of entry wasd (%s)\r\n", entry);
        free(entry);
    }

    printf("\r\n");

    main_conf = config_entry_set(main_conf, "wasd2.cap", "youisstupid", NULL);
    printf("removed entry youisstupid\r\n");
    if(config_entry_exists(main_conf, "wasd2.cap", "youisstupid") == 1)
    {
        printf("got entry exists youisstupid\r\n");
        char *entry = config_entry_get(main_conf, NULL, "youisstupid");
        printf("got value of entry youisstupid (%s)\r\n", entry);
        free(entry);
    }

    printf("\r\n");

    free_config(&main_conf);
    
    return 0;
}