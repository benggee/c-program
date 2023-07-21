#ifndef __TCP_SERVER_H__
#define __TCP_SERVER_H__

#include "common.h"

struct worker_thread_context {
    int fd;
    pthread_t thread_id;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
};

int create_tcp_socket(int port);

#endif