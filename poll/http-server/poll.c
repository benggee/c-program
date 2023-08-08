#include <poll.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>

#include "poll.h"
#include "handle_msg.h"

struct poll_dispatcher *poll_new() {
    struct poll_dispatcher *poll_dis = malloc(sizeof(struct poll_dispatcher));
    poll_dis->size = MAX_POLL_SIZE;
    poll_dis->fds = malloc(sizeof(struct pollfd) * MAX_POLL_SIZE);

    pthread_cond_init(&poll_dis->cond, NULL);
    pthread_mutex_init(&poll_dis->mutex, NULL);

    if (socketpair(AF_UNIX, SOCK_STREAM, 0, poll_dis->pair) < 0) {
        perror("socketpair set fialed");
    }

    poll_dis->fds[0].fd = poll_dis->pair[1];
    poll_dis->fds[0].events = POLLRDNORM;

    int i;
    for (i = 1; i < MAX_POLL_SIZE; i++) {
        poll_dis->fds[i].fd = -1;
    }

    poll_dis->done = 0;
    poll_dis->buffer_map = new_buffer_map();

    return poll_dis;
}

int poll_add(struct poll_dispatcher *poll_dis, int fd) {
    int i;
    for (i = 0; i < poll_dis->size; i++) {
        if (poll_dis->fds[i].fd == -1) {
            poll_dis->fds[i].fd = fd;
            poll_dis->fds[i].events = POLLRDNORM;

            char c = 'a';
            ssize_t n= write(poll_dis->pair[0], &c, sizeof(c));
            if (n != sizeof(c)) {
                perror("write error");
            }

            printf("poll-add-fd: %d\n", fd);
            return 0;
        }
    }

    perror("too many clients");

    return -1;
}

int poll_del(struct poll_dispatcher *poll_dis, int fd) {
    int i;
    for (i = 0; i < MAX_POLL_SIZE; i++) {
        if (poll_dis->fds[i].fd == fd) {
            poll_dis->fds[i].fd = -1;
            return 0;
        }
    }

    perror("can't find fd");

    return -1;
}

int poll_dispatch(struct poll_dispatcher *poll_dis) {
    int ret, i;

    pthread_mutex_lock(&poll_dis->mutex);

    poll(poll_dis->fds, poll_dis->size, 0);
    poll_dis->done = 1;

    pthread_cond_signal(&poll_dis->cond);
    pthread_mutex_unlock(&poll_dis->mutex);

    while(1) {
        ret = poll(poll_dis->fds, poll_dis->size, -1);
        if (ret == -1) {
            perror("poll error");
            return -1;
        }

        printf("poll-returned: %d\n", ret);

        for (i = 0; i < poll_dis->size; i++) {
            if (poll_dis->fds[i].fd == -1) {
                continue;
            }

            // wakeup
            if (poll_dis->fds[i].fd == poll_dis->pair[1] && poll_dis->fds[i].revents & POLLRDNORM) {
                char c;
                ssize_t n = read(poll_dis->pair[1], &c, sizeof(c));
                if (n != sizeof(c)) {
                    perror("read error");
                }
                printf("wakeup\n");
                break;
            }

            if (poll_dis->fds[i].revents & (POLLRDNORM | POLLERR)) {
                int client_fd = poll_dis->fds[i].fd;
               do_msg(poll_dis->buffer_map->s_buffer[client_fd], client_fd);
            }
        }
   }
}

void poll_free(struct poll_dispatcher *poll_dis) {
    free(poll_dis->fds);
    free_buffer_map(poll_dis->buffer_map);
    poll_dis->buffer_map = NULL;
    free(poll_dis);
}