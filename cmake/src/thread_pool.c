#include "thread_pool.h"
#include "tcp_server.h"
#include "reader.h"
#include "http_response.h"

void *worker(void *arg) {
    struct worker_thread_context *ctx = (struct worker_thread_context *) arg;

    while (1) {
        pthread_mutex_lock(&ctx->mutex);

        pthread_cond_wait(&ctx->cond, &ctx->mutex);

        if (ctx->fd == -1) {
            perror("invalid fd");
            break;
        }

        printf("THREAD-ID: %ld; FD: %d\n", ctx->thread_id, ctx->fd);
        char buf[1024];
        read_line(ctx->fd, buf, sizeof(buf));
        printf("%s", buf);
        memset(buf, 0, sizeof(buf));
        read_line(ctx->fd, buf, sizeof(buf));
        printf("%s", buf);
        memset(buf, 0, sizeof(buf));
        read_line(ctx->fd, buf, sizeof(buf));
        printf("%s", buf);

        write_response(ctx->fd);

        pthread_mutex_unlock(&ctx->mutex);
    }
}

void *accepter_thread(void *arg) {
    int fd = *(int *)arg;

#ifdef WORKER_POOL_SIZE
    struct worker_thread_context *ctx = malloc(WORKER_POOL_SIZE * sizeof(struct worker_thread_context));

    for (int i = 0; i < WORKER_POOL_SIZE; i++) {
        pthread_mutex_init(&ctx[i].mutex, NULL);
        pthread_cond_init(&ctx[i].cond, NULL);
        ctx[i].fd = -1;

        pthread_create(&ctx[i].thread_id, NULL, worker, (void *)&ctx[i]);
    }
#endif

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
#ifdef WORKER_POOL_SIZE
        if (idx + 1 == WORKER_POOL_SIZE) {
            idx = 0;
        }

        pthread_mutex_lock(&ctx[idx].mutex);

        ctx[idx].fd = conn_fd;

        pthread_mutex_unlock(&ctx[idx].mutex);
        pthread_cond_signal(&ctx[idx].cond);

        idx++;
#else
        char buf[1024];
        read_line(conn_fd, buf, sizeof(buf));
        printf(buf);
        memset(buf, 0, sizeof(buf));
        read_line(conn_fd, buf, sizeof(buf));
        printf(buf);
        memset(buf, 0, sizeof(buf));
        read_line(conn_fd, buf, sizeof(buf));
        printf(buf);

        write_response(conn_fd);
#endif
    }
}