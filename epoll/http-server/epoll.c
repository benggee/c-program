#include <poll.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>

#include "epoll.h"
#include "handle_msg.h"

struct epoll_dispatcher *epoll_new() {
    struct epoll_dispatcher *poll_dis = malloc(sizeof(struct epoll_dispatcher));
    poll_dis->size = MAX_POLL_SIZE;
    poll_dis->events = calloc(MAX_POLL_SIZE, sizeof(struct epoll_event));
    poll_dis->epfd = epoll_create1(0);

    pthread_cond_init(&poll_dis->cond, NULL);
    pthread_mutex_init(&poll_dis->mutex, NULL);

    if (socketpair(AF_UNIX, SOCK_STREAM, 0, poll_dis->pair) < 0) {
        perror("socketpair set fialed");
    }

    struct epoll_event ev;
    ev.data.fd = poll_dis->pair[1];
    ev.events = EPOLLIN;
    epoll_ctl(poll_dis->epfd, EPOLL_CTL_ADD, poll_dis->pair[1], &ev);

    poll_dis->done = 0;
    poll_dis->buffer_map = new_buffer_map();

    return poll_dis;
}

int epoll_add(struct epoll_dispatcher *poll_dis, int fd) {
    struct epoll_event ev;
    ev.data.fd = fd;
    ev.events = EPOLLIN;

    epoll_ctl(poll_dis->epfd, EPOLL_CTL_ADD, fd, &ev);

    return 0;
}

int epoll_del(struct epoll_dispatcher *poll_dis, int fd) {
    struct epoll_event ev;
    ev.data.fd = fd;
    ev.events = EPOLLIN;

    epoll_ctl(poll_dis->epfd, EPOLL_CTL_DEL, fd, &ev);

    return 0;
}

int epoll_dispatch(struct epoll_dispatcher *poll_dis) {
    int ret, i;

    pthread_mutex_lock(&poll_dis->mutex);

    epoll_wait(poll_dis->epfd, poll_dis->events, poll_dis->size, 0);

    poll_dis->done = 1;

    pthread_cond_signal(&poll_dis->cond);
    pthread_mutex_unlock(&poll_dis->mutex);

    while(1) {
        ret = epoll_wait(poll_dis->epfd, poll_dis->events, poll_dis->size, -1);

        for (i = 0; i < ret; i++) {
            if (poll_dis->events[i].data.fd == poll_dis->pair[1]) {
                char c;
                ssize_t n = read(poll_dis->pair[1], &c, sizeof(c));
                if (n != sizeof(c)) {
                    perror("read error");
                }
                printf("wakeup\n");
                continue;
            }

            int client_fd = poll_dis->events[i].data.fd;
            do_msg(poll_dis->buffer_map->s_buffer[client_fd], client_fd);
        }
   }
}

void epoll_free(struct epoll_dispatcher *poll_dis) {
    free(poll_dis->events);
    free_buffer_map(poll_dis->buffer_map);
    poll_dis->buffer_map = NULL;
    free(poll_dis);
}