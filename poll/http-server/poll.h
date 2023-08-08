#ifndef __POLL_H__
#define __POLL_H__

#include <pthread.h>

#define MAX_POLL_SIZE 1024

struct poll_dispatcher {
    int size;   // max fd nums
    int done;    // is done 0 not done  1 done
    int pair[2];
    pthread_cond_t cond;
    pthread_mutex_t mutex;
    struct pollfd *fds;
    struct buffer_map *buffer_map;
};

struct poll_dispatcher *poll_new();

int poll_add(struct poll_dispatcher *poll_dis, int fd);

int poll_del(struct poll_dispatcher *poll_dis, int fd);

int poll_dispatch(struct poll_dispatcher *poll_dis);

void poll_free(struct poll_dispatcher *poll_dis);

#endif