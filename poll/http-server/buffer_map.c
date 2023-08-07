#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "buffer.h"
#include "buffer_map.h"

struct sock_buffer *new_sock_buffer() {
    struct sock_buffer *s_buf = malloc(sizeof(struct sock_buffer));
    if (s_buf == NULL) {
        return NULL;
    }
    s_buf->r_buffer = new_buffer();
    s_buf->w_buffer = new_buffer();
    return s_buf;
}

struct buffer_map *new_buffer_map() {
    struct buffer_map *map = malloc(sizeof(struct buffer_map));
    if (map == NULL) {
        return NULL;
    }
    map->max_idx = 0;
    map->s_buffer = NULL;
    return map;
}

int allocation_buffer_map(struct buffer_map *map, int slot) {
    if (map->max_idx > slot) {
        map->s_buffer[slot] = new_sock_buffer();
        return 0;
    }

    int max_idx = map->max_idx ? map->max_idx : 32;
    struct sock_bufferr **tmp = NULL;

    while(max_idx <= slot) {
        max_idx <<= 1;  // max_idx = max_idx * 2
    }

    tmp = realloc(map->s_buffer, max_idx * sizeof(struct sock_buffer *));
    if (tmp == NULL) {
        return -1;
    }

    struct sock_buffer *s_buf = new_sock_buffer();
    if (s_buf == NULL) {
        return -1;
    }

    tmp[slot] = s_buf;

    map->max_idx = max_idx;
    map->s_buffer = tmp;

    return 0;
}

void free_buffer_map(struct buffer_map *map) {
    if (map->max_idx == NULL) {
        return;
    }

    int i;
    for (i = 0; i < map->max_idx; ++i) {
        if (map->s_buffer[i] != NULL) {
            free(map->s_buffer[i]);
        }
        free(map->s_buffer);
        map->s_buffer = NULL;
    }
    map->s_buffer = 0;
}