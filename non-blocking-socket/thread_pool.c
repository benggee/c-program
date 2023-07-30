#include <errno.h>
#include "thread_pool.h"
#include "tcp_server.h"
#include "buffer.h"
#include "buffer_map.h"

void make_nonblocking(int fd) {
    fcntl(fd, F_SETFL, O_NONBLOCK);
}

void handle_msg(struct sock_buffer *s_buffer, int sock_fd) {
    if (s_buffer == NULL) {
        perror("invalid sock_buffer");
        return;
    }

    struct buffer *r_buffer = s_buffer->r_buffer;
    while(1) {
        int ret = buffer_read_from_socket(r_buffer, sock_fd);
        if (ret == -1) {
            if (errno == EAGAIN) {
                continue;
            }
            perror("read error");
            break;
        }
        if (ret == 0) {
            close(sock_fd);
            break;
        }

        if (ret > 0) {
            printf("read %d bytes, ret: %s\n", ret, r_buffer->data);
        }
    }
}

void *worker(void *arg) {
    struct worker_thread_context *ctx = (struct worker_thread_context *) arg;

    while (1) {
        pthread_mutex_lock(&ctx->mutex);

        pthread_cond_wait(&ctx->cond, &ctx->mutex);

        if (ctx->fd == -1) {
            perror("invalid fd");
            break;
        }

        struct sock_buffer *s_buffer = ctx->buffer_map->s_buffer[ctx->fd];

        handle_msg(s_buffer, ctx->fd);

        pthread_mutex_unlock(&ctx->mutex);
    }
}

void *accepter_thread(void *arg) {
    int fd = *(int *)arg;

    struct buffer_map *map = new_buffer_map();
    if (map == NULL) {
        perror("new_buffer_map error");
        exit(1);
    }

    struct worker_thread_context *ctx = malloc(WORKER_POOL_SIZE * sizeof(struct worker_thread_context));
    ctx->buffer_map = map;

    for (int i = 0; i < WORKER_POOL_SIZE; i++) {
        pthread_mutex_init(&ctx[i].mutex, NULL);
        pthread_cond_init(&ctx[i].cond, NULL);
        ctx[i].fd = -1;

        pthread_create(&ctx[i].thread_id, NULL, worker, (void *)&ctx[i]);
    }

    struct sockaddr_in client_addr;

    int idx = 0;
    while(1) {
        bzero(&client_addr, sizeof(client_addr));
        socklen_t len = sizeof(client_addr);
        int conn_fd = accept(fd, (struct sockaddr *)&client_addr, &len);
        if (conn_fd < 0) {
            perror("accept error");
            break;
        }

        make_nonblocking(conn_fd);

        if (allocation_buffer_map(map, conn_fd) < 0) {
            perror("allocation_buffer_map error");
            break;
        }


        if (idx + 1 == WORKER_POOL_SIZE) {
            idx = 0;
        }

        pthread_mutex_lock(&ctx[idx].mutex);

        ctx[idx].fd = conn_fd;

        pthread_mutex_unlock(&ctx[idx].mutex);
        pthread_cond_signal(&ctx[idx].cond);

        idx++;
    }
}