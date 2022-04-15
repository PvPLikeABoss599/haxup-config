#pragma once

void util_zero(void *ptr, int size);
void util_cpy(void *to_ptr, void *ptr, int start, int stop);
int util_get_white_end(void *ptr, int size);
int util_get_char_count(void *ptr, char delim);
