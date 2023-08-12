#ifndef __POLL_H__
#define __POLL_H__

#include <pthread.h>
#include <sys/epoll.h>

#define MAX_POLL_SIZE 1024

struct epoll_dispatcher {
    int size;   // max fd nums
    int done;    // is done 0 not done  1 done
    int epfd;
    int pair[2];
    pthread_cond_t cond;
    pthread_mutex_t mutex;
    struct epoll_event *events;
    struct buffer_map *buffer_map;
};

struct epoll_dispatcher *epoll_new();

int epoll_add(struct epoll_dispatcher *poll_dis, int fd);

int epoll_del(struct epoll_dispatcher *poll_dis, int fd);

int epoll_dispatch(struct epoll_dispatcher *poll_dis);

void epoll_free(struct epoll_dispatcher *poll_dis);

#endif