#ifndef __BUFFER_MAP_H__
#define __BUFFER_MAP_H__

#include <stdlib.h>
#include "buffer.h"

struct sock_buffer {
    struct buffer *w_buffer;
    struct buffer *r_buffer;
};

struct buffer_map {
    struct sock_buffer **s_buffer;
    int max_idx;
};

struct sock_buffer *new_sock_buffer();

struct buffer_map *new_buffer_map();

int allocation_buffer_map(struct buffer_map *map, int slot);

void free_buffer_map(struct buffer_map *map);

#endif