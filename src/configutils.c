
#include <stdint.h>

#include "haxup.h"
#include "configutils.h"

void util_zero(void *ptr, int size)
{
    uint8_t *ptr_p = ptr; 
    int j;
    for(j = 0; j < size; j++)
    {
        ptr_p[j] = 0;
    }
    return;
}


void util_cpy(void *to_ptr, void *ptr, int start, int stop)
{
    uint8_t *ptr_p = ptr; 
    uint8_t *to_ptr_p = to_ptr; 
    int to_pos = 0;
    int j;
    for(j = start; j <= stop; j++)
    {
        to_ptr_p[to_pos] = ptr_p[j];
        to_pos++;
    }
    return;
}

int util_get_white_end(void *ptr, int size)
{
    uint8_t *ptr_p = ptr;
    int pos, ret = 0;
    while((ptr_p[ret]) == ' ' || ((ptr_p[ret]) == '\t'))
    {
        ret++;
    }
    return ret;
}

int util_get_char_count(void *ptr, char delim)
{
    uint8_t *ptr_p = ptr;
    int pos = 0, ret = 0;
    while(ptr_p[pos] != '\0')
    {
        if(ptr_p[pos] == delim) ret++;
        pos++;
    }
    return ret;
}
