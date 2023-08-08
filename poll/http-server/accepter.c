#include <fcntl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <string.h>
#include <stdio.h>

#include "accepter.h"
#include "poll.h"
#include "tcp_server.h"

void *worker(void *arg) {
    struct poll_dispatcher *poll_dis = (struct poll_dispatcher *)arg;

    printf("poll-size: %d\n", poll_dis->size);

    poll_dispatch(poll_dis);

    poll_free(poll_dis);
}

void accept_sock(int serv_fd) {
    int conn_fd;

    struct poll_dispatcher *poll_dis = poll_new();

//    make_nonblocking(serv_fd);

    struct sockaddr_in client_addr;
    socklen_t cli_len = sizeof(client_addr);

    pthread_t thread_id;

    pthread_create(&thread_id, NULL, worker, (void *)poll_dis);

    pthread_mutex_lock(&poll_dis->mutex);
    while(!poll_dis->done)
        pthread_cond_wait(&poll_dis->cond, &poll_dis->mutex);
    pthread_mutex_unlock(&poll_dis->mutex);

    while(1) {
        memset(&client_addr, 0, sizeof(client_addr));

        conn_fd = accept(serv_fd, (struct sockaddr *)&client_addr, &cli_len);
        if (conn_fd < 0) {
            perror("accept error");
            continue;
        }

        make_nonblocking(conn_fd);

        printf("accept a new connection: %d\n", conn_fd);

        if (allocation_buffer_map(poll_dis->buffer_map, conn_fd) < 0) {
            perror("allocation_buffer_map error");
            continue;
        }

        poll_add(poll_dis, conn_fd);
    }
}

void make_nonblocking(int fd) {
    fcntl(fd, F_SETFL, O_NONBLOCK);
}