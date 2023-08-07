#ifndef __TCP_SERVER_H__
#define __TCP_SERVER_H__

#include "common.h"
#include "buffer_map.h"

struct worker_thread_context {
    int fd;
    pthread_t thread_id;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    struct buffer_map *buffer_map;
};

int create_tcp_socket(int port);

#endif