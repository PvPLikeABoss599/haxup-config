#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>

#include "haxup.h"
#include "config.h"
#include "configutils.h"

void free_config(struct config_t **cfg)
{
    int a = 0;
    
    for(a = 0; a < (*cfg)->entries_len; a++)
    {
        free((*cfg)->entries[a]);
        (*cfg)->entries[a] = NULL;
    }
    free((*cfg)->entries);
    (*cfg)->entries = NULL;

    for(a = 0; a < (*cfg)->parents_len; a++)
    {
        free((*cfg)->parents[a]);
        (*cfg)->parents[a] = NULL;
    }
    free((*cfg)->parents);
    (*cfg)->parents = NULL;

    (*cfg)->entries_len = 0;
    (*cfg)->parents_len = 0;

    (*cfg)->in_group = 0;

    util_zero((*cfg)->location, 1024);

    free(*cfg);
    *cfg = NULL;
}

struct config_t *load_config(char *path)
{
    struct config_t *config = malloc(sizeof(struct config_t));

    struct config_entry_parent_t *curr = NULL;

    util_zero(config->location, 1024);
    util_cpy(config->location, path, 0, strlen(path));
    config->in_group = 0;
    config->entries = NULL;
    config->entries_len = 0;
    config->parents = NULL;
    config->parents_len = 0;

    FILE *fp = fopen(path, "r");
    if(!fp)
    {
        free(config);
        config = NULL;
        return config;
    }

    unsigned char *buffer = malloc(1024);
    util_zero(buffer, 1024);
    while(fgets(buffer, 1024, fp) != NULL)
    {
        int pos, first_parent = 0;

        for(pos = 0; pos < 1024; pos++)
        {
            if(buffer[pos] == '\r')
            {
                buffer[pos] = 0;
                continue;
            }
            if(buffer[pos] == '\n')
            {
                buffer[pos] = 0;
                continue;
            }
        }

        //check for larger group
        for(pos = 0; pos < 1024; pos++)
        {
            if(buffer[pos] == '[')
            {
                config->in_group++;
                first_parent = 1;
                break;
            }
        }

        // get parent of group
        if(config->in_group > 0 && first_parent == 1)
        {
            config->parents = realloc(config->parents, (config->parents_len + 1) * sizeof(struct config_entry_parent_t *));
            config->parents[config->parents_len] = malloc(sizeof(struct config_entry_parent_t));
            struct config_entry_parent_t *parent = config->parents[config->parents_len];
            config->parents_len++;

            parent->next = NULL;
            parent->prev = NULL;
            util_zero(parent->parent_path, 1024);

            uint8_t end = 0;
            for(pos = 0; pos < 1024; pos++)
            {
                if(buffer[pos] == ':')
                {
                    if(curr == NULL)
                    {
                        util_cpy(parent->parent_path, buffer, util_get_white_end(buffer, 1024), pos-1);
                        parent->prev = NULL;
                        curr = parent;
                    }
                    else
                    {
                        int parent_path_len = 0;
                        util_cpy(parent->parent_path, curr->parent_path, 0, strlen(curr->parent_path));
                        parent_path_len = strlen(parent->parent_path);
                        util_cpy(parent->parent_path+parent_path_len, ".", 0, 1);
                        parent_path_len = strlen(parent->parent_path);
                        util_cpy(parent->parent_path+parent_path_len, buffer, util_get_white_end(buffer, 1024), pos-1);
                        parent->prev = curr;
                        curr = parent;
                    }
                    first_parent = 0;
                    break;
                }
            }
        }
        else if (config->in_group > 0)// process children of a parent
        {
            for(pos = 0; pos < 1024; pos++)
            {
                if(buffer[pos] == ']')
                {
                    config->in_group--;
                    if(curr != NULL && curr->prev != NULL)
                    {
                        curr = curr->prev;
                    }
                    else
                    {
                        curr = NULL;
                    }
                    break;
                }
                if(buffer[pos] == ':')
                {
                    config->entries = realloc(config->entries, (config->entries_len+1) * sizeof(struct config_entry_t *));
                    config->entries[config->entries_len] = malloc(sizeof(struct config_entry_t));
                    struct config_entry_t *entry = config->entries[config->entries_len];
                    config->entries_len++;
                    entry->parent = curr;
                    util_zero(entry->location, 1024);
                    util_zero(entry->value, 1024);
                    util_cpy(entry->location, buffer, util_get_white_end(buffer, 1024), pos-1);
                    util_cpy(entry->value, buffer, pos+2, 256);
                    #ifdef DEBUG
                    printf("whitespace is %d\r\n", util_get_white_end(buffer, 1024));
                    #endif
                    break;
                }
            }
        }
        else
        {
            for(pos = 0; pos < 1024; pos++)
            {
                if(buffer[pos] == ':')
                {
                    config->entries = realloc(config->entries, (config->entries_len+1) * sizeof(struct config_entry_t *));
                    config->entries[config->entries_len] = malloc(sizeof(struct config_entry_t));
                    struct config_entry_t *entry = config->entries[config->entries_len];
                    config->entries_len++;
                    entry->parent = NULL;
                    util_zero(entry->location, 1024);
                    util_zero(entry->value, 1024);
                    util_cpy(entry->location, buffer, util_get_white_end(buffer, 1024), pos-1);
                    util_cpy(entry->value, buffer, pos+2, 256);
                    break;
                }
            }
        }

        util_zero(buffer, 1024);
    }

    fclose(fp);
    fp = NULL;

    #ifdef DEBUG
    int x = 0;
    for(x = 0; x < config->parents_len; x++)
    {
        printf("have group %d-(%s)\r\n", x, config->parents[x]->parent_path);
    }

    for(x = 0; x < config->entries_len; x++)
    {
        if(config->entries[x]->parent == NULL)
        {
            printf("have root entry %d-(%s)-(%s)\r\n", x, config->entries[x]->location, config->entries[x]->value);
        }
        else 
        {
            printf("have entry in group %d-(%s:%s)-(%s)\r\n", x, config->entries[x]->parent->parent_path, config->entries[x]->location, config->entries[x]->value);
        }
    }
    #endif

    free(buffer);
    buffer = NULL;

    return config;
}

uint8_t config_entry_exists(struct config_t *cfg, char *path, char *entry)
{
    int x;
    if(path != NULL)
    {
        for(x = 0; x < cfg->parents_len; x++)
        {
            if(strcmp(cfg->parents[x]->parent_path, path) == 0)
            {
                struct config_entry_parent_t *parent;
                parent = cfg->parents[x];
                for(x = 0; x < cfg->entries_len; x++)
                {
                    if(parent == cfg->entries[x]->parent)
                    {
                        if(strcmp(cfg->entries[x]->location, entry) == 0)
                        {
                            return 1;
                        }
                    }
                }
            }
        }
    }
    else
    {
        for(x = 0; x < cfg->entries_len; x++)
        {
            if(NULL == cfg->entries[x]->parent)
            {
                if(strcmp(cfg->entries[x]->location, entry) == 0)
                {
                    return 1;
                }
            }
        }
    }
    return 0;
}

uint8_t *config_entry_get(struct config_t *cfg, char *path, char *entry)
{
    int x;
    if(path != NULL)
    {
        for(x = 0; x < cfg->parents_len; x++)
        {
            if(strcmp(cfg->parents[x]->parent_path, path) == 0)
            {
                struct config_entry_parent_t *parent;
                parent = cfg->parents[x];
                for(x = 0; x < cfg->entries_len; x++)
                {
                    if(parent == cfg->entries[x]->parent)
                    {
                        if(strcmp(cfg->entries[x]->location, entry) == 0)
                        {
                            uint8_t *ret = malloc(strlen(cfg->entries[x]->value)+2);
                            util_zero(ret, strlen(cfg->entries[x]->value)+2);
                            util_cpy(ret, cfg->entries[x]->value, 0, strlen(cfg->entries[x]->value));
                            util_cpy(ret+strlen(ret), "\0", 0, 1);
                            return ret;
                        }
                    }
                }
            }
        }
    }
    else
    {
        for(x = 0; x < cfg->entries_len; x++)
        {
            if(NULL == cfg->entries[x]->parent)
            {
                if(strcmp(cfg->entries[x]->location, entry) == 0)
                {
                    uint8_t *ret = malloc(strlen(cfg->entries[x]->value)+2);
                    util_zero(ret, strlen(cfg->entries[x]->value)+2);
                    util_cpy(ret, cfg->entries[x]->value, 0, strlen(cfg->entries[x]->value));
                    util_cpy(ret+strlen(ret), "\0", 0, 1);
                    return ret;
                }
            }
        }
    }
    uint8_t *ret = malloc(5);
    util_cpy(ret, "NULL\0", 0, 5);
    return 0;
}

uint8_t **config_entry_get_section(struct config_t *cfg, char *path, int *_ret_len)
{
    uint8_t **ret = NULL;
    int x, ret_len = 0;
    if(path != NULL)
    {
        for(x = 0; x < cfg->entries_len; x++)
        {
            if(cfg->entries[x]->parent == NULL) continue;
            if(strcmp(cfg->entries[x]->parent->parent_path, path) == 0)
            {
                ret = realloc(ret, (ret_len+1)*sizeof(uint8_t *));
                ret[ret_len] = malloc(strlen(cfg->entries[x]->location)+1);
                uint8_t *str = ret[ret_len];
                strcpy(str, cfg->entries[x]->location);
                str[strlen(cfg->entries[x]->location)] = 0;
                ret_len++;
            }
        }
    }
    *_ret_len = ret_len;
    return ret;
}

struct config_t *config_entry_set(struct config_t *cfg, char *path, char *entry_str, uint8_t *value)
{
    struct config_entry_parent_t *entry_parent = NULL;
    struct config_entry_parent_t *curr = NULL;
    FILE *fp = fopen(cfg->location, "r");
    if(!fp)
    {
        return cfg;
    }

    char newfile[1024];
    util_cpy(newfile, cfg->location, 0, 1024);
    util_cpy(newfile+strlen(newfile), ".tmp", 0, 4);
    FILE *newfp = fopen(newfile, "w+");
    if(!newfp)
    {
        return cfg;
    }

    if(path != NULL)
    {
        entry_parent = malloc(sizeof(struct config_entry_parent_t));
        entry_parent->prev = NULL;
        entry_parent->next = NULL;
        entry_parent->value_indent = util_get_char_count(path, '.') * 5;
        util_cpy(entry_parent->parent_path, path, 0, strlen(path));
    }

    uint8_t replaced = 0;
    uint8_t flush = 1;
    int in_group = 0;
    int group_count = 0;

    char *buffer = malloc(1024);
    util_zero(buffer, 1024);
    while(fgets(buffer, 1024, fp) != NULL)
    {
        int pos, first_parent = 0;

        for(pos = 0; pos < 1024; pos++)
        {
            if(buffer[pos] == '\r')
            {
                buffer[pos] = 0;
                continue;
            }
            if(buffer[pos] == '\n')
            {
                buffer[pos] = 0;
                continue;
            }
        }

        //check for larger group
        for(pos = 0; pos < 1024; pos++)
        {
            if(buffer[pos] == '[')
            {
                in_group++;
                group_count++;
                first_parent = 1;
                break;
            }
        }

        // get parent of group
        if(in_group > 0 && first_parent == 1)
        {
            struct config_entry_parent_t *parent = malloc(sizeof(struct config_entry_parent_t));

            parent->prev = NULL;
            parent->next = NULL;
            util_zero(parent->parent_path, 1024);
            parent->value_indent = 0;

            uint8_t end = 0;
            for(pos = 0; pos < 1024; pos++)
            {
                if(buffer[pos] == ':')
                {
                    if(curr == NULL)
                    {
                        util_cpy(parent->parent_path, buffer, util_get_white_end(buffer, 1024), pos-1);
                        parent->prev = NULL;
                        curr = parent;
                        parent->value_indent = 1;
                    }
                    else
                    {
                        int parent_path_len = 0;
                        util_cpy(parent->parent_path, curr->parent_path, 0, strlen(curr->parent_path));
                        parent_path_len = strlen(parent->parent_path);
                        util_cpy(parent->parent_path+parent_path_len, ".", 0, 1);
                        parent_path_len = strlen(parent->parent_path);
                        util_cpy(parent->parent_path+parent_path_len, buffer, util_get_white_end(buffer, 1024), pos-1);
                        parent->value_indent += curr->value_indent+1;
                        parent->prev = curr;
                        curr = parent;
                        #ifdef DEBUG
                        printf("Got Parent %s\r\n", parent->parent_path);
                        #endif
                    }
                    first_parent = 0;
                    break;
                }
            }
        }
        else if (in_group > 0)// process children of a parent
        {
            for(pos = 0; pos < 1024; pos++)
            {
                if(buffer[pos] == ']')
                {
                    if(replaced == 0 && curr != NULL && path != NULL)
                    {
                        if(strcmp(curr->parent_path, path) == 0 && value != NULL)
                        {
                            char tmp[512];
                            util_zero(tmp, 512);
                            int x;
                            for(x = 0; x < curr->value_indent*5; x++)
                            {
                                util_cpy(tmp+strlen(tmp), " ", 0, 1);
                            }
                            fprintf(newfp, "%s%s: %s\r\n", tmp, entry_str, value);
                            fflush(newfp);
                            replaced = 1;
                            flush = 1;
                            #ifdef DEBUG
                            printf("[3] write %s:%s with %s\r\n", path, entry_str, value);
                            #endif
                        }
                    }
                    in_group--;
                    if(curr != NULL && curr->prev != NULL)
                    {
                        curr->prev->next = curr;
                        curr = curr->prev;
                        free(curr->next);
                    }
                    else if(curr != NULL && curr->prev == NULL)
                    {
                        free(curr);
                        curr = NULL;
                    }
                    else
                    {
                        curr = NULL;
                    }
                    break;
                }
                if(buffer[pos] == ':')
                {
                    struct config_entry_t *entry = malloc(sizeof(struct config_entry_t));
                    entry->parent = curr;

                    util_zero(entry->location, 1024);
                    util_zero(entry->value, 1024);
                    util_cpy(entry->location, buffer, util_get_white_end(buffer, 1024), pos-1);
                    util_cpy(entry->value, buffer, pos+2, 1024-pos);
                    if(path != NULL && strcmp(path, entry->parent->parent_path) == 0)
                    {
                        if(strcmp(entry->location, entry_str) == 0)
                        {
                            if(value == NULL)
                            {
                                replaced = 1;
                                flush = 0;
                            }
                            else 
                            {
                                int whitespace = util_get_white_end(buffer, 1024);
                                int a = 0;
                                for(a = 0; a < whitespace; a++)
                                {
                                    fprintf(newfp, " ");
                                }
                                fprintf(newfp, "%s: %s\r\n", entry->location, value);
                                fflush(newfp);
                                replaced = 1;
                                flush = 0;
                                #ifdef DEBUG
                                printf("[1] replace %s:%s with %s\r\n", path, entry->location, value);
                                #endif
                            }
                        }
                    }
                    free(entry);
                    entry = NULL;
                    break;
                }
            }
        }
        else
        {
            for(pos = 0; pos < 1024; pos++)
            {
                if(buffer[pos] == ':')
                {
                    struct config_entry_t *entry = malloc(sizeof(struct config_entry_t));
                    entry->parent = NULL;

                    util_zero(entry->location, 1024);
                    util_zero(entry->value, 1024);

                    util_cpy(entry->location, buffer, util_get_white_end(buffer, 1024), pos-1);
                    util_cpy(entry->value, buffer, pos+2, 256);
                    if(path == NULL)
                    {
                        if(strcmp(entry->location, entry_str) == 0)
                        {
                            if(value == NULL)
                            {
                                replaced = 1;
                                flush = 0;
                            }
                            else 
                            {
                                int whitespace = util_get_white_end(buffer, 1024);
                                int a = 0;
                                for(a = 0; a < whitespace; a++)
                                {
                                    fprintf(newfp, " ");
                                }
                                fprintf(newfp, "%s: %s\r\n", entry->location, value);
                                fflush(newfp);
                                replaced = 1;
                                flush = 0;
                                #ifdef DEBUG
                                printf("[2] replace %s:%s with %s\r\n", path, entry->location, value);
                                #endif
                            }
                        }
                    }
                    free(entry);
                    entry = NULL;
                    break;
                }
            }
        }
        if(flush == 1)
        {
            fprintf(newfp, "%s\r\n", buffer);
            fflush(newfp);
        }
        else
        {
            flush = 1;
        }
        util_zero(buffer, 1024);
    }

    if(replaced == 0)
    {
        if(path == NULL && value != NULL)
        {
            fprintf(newfp, "%s: %s\r\n", entry_str, value);
            fflush(newfp);
            #ifdef DEBUG
            printf("[4] replace %s with %s\r\n", entry_str, value);
            #endif
        }
        else
        {

        }
    }

    util_zero(buffer, 1024);
    free(buffer);
    buffer = NULL;

    fclose(fp);
    fclose(newfp);
    unlink(cfg->location);
    rename(newfile, cfg->location);

    char cfg_path[1024];
    util_cpy(cfg_path, cfg->location, 0, strlen(cfg->location));

    free_config(&cfg);

    cfg = load_config(cfg_path);

    return cfg;
}
