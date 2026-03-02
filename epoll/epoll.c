/**
 * @file epoll.c
 * @brief 基于 epoll 的 I/O 多路复用 TCP 服务器
 *
 * Epoll 是 Linux 特有的高性能 I/O 事件通知机制，
 * 专为处理大量并发连接而设计。
 *
 * 核心优势：
 * - O(1) 时间复杂度：只处理就绪的 FD
 * - 支持大规模连接：可轻松处理 10 万+ 连接
 * - 边缘触发 (ET)：减少系统调用次数
 *
 * 三大核心函数：
 * - epoll_create1(): 创建 epoll 实例
 * - epoll_ctl(): 添加/修改/删除监控的 FD
 * - epoll_wait(): 等待事件发生
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/epoll.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <errno.h>
#include <fcntl.h>

#define MAX_EVENTS 1024  // epoll_wait 最多返回的事件数

/**
 * @brief 创建并配置监听 Socket
 * @return 成功返回 Socket FD，失败返回 -1
 */
int create_sock();

/**
 * @brief 将 Socket 设置为非阻塞模式
 * @param fd Socket 文件描述符
 *
 * 注意：边缘触发 (ET) 模式必须配合非阻塞 I/O 使用
 */
void make_nonblocking(int fd);

int main(int argc, char *argv[]) {
    int read_num, conn_fd, n, efd, i;
    char buf[1024];
    struct sockaddr_in client_addr;

    int sock_fd = create_sock();

    make_nonblocking(sock_fd);

    struct epoll_event event;
    struct epoll_event *events;

    efd = epoll_create1(0);
    if (efd == -1) {
        perror("epoll_create1 failed.");
        exit(1);
    }

    event.data.fd = sock_fd;
    event.events = EPOLLIN | EPOLLET;  // 边缘触发
    if (epoll_ctl(efd, EPOLL_CTL_ADD, sock_fd, &event) == -1) {
        perror("epoll_ctl failed.");
        exit(1);
    }

    events = calloc(MAX_EVENTS, sizeof(event));

    for (;;) {
        read_num = epoll_wait(efd, events, MAX_EVENTS, -1);

        for (i = 0; i < read_num; i++) {
            if ((events[i].events & EPOLLERR) ||
                (events[i].events & EPOLLHUP ||
                (!(events[i].events & EPOLLIN))))
            {
                perror("epoll error.");
                close(events[i].data.fd);
                continue;
            } else if (sock_fd == events[i].data.fd) {
                socklen_t cli_len = sizeof(client_addr);
                conn_fd = accept(sock_fd, (struct sockaddr *)&client_addr, &cli_len);
                if (conn_fd == -1) {
                    perror("accept error.");
                    continue;
                } else {
                    event.data.fd = conn_fd;
                    event.events = EPOLLIN | EPOLLET;
                    if (epoll_ctl(efd, EPOLL_CTL_ADD, conn_fd, &event) == -1) {
                        perror("epoll_ctl error.");
                    }
                }
                continue;
            } else {
                int client_fd = events[i].data.fd;

                if (n = read(client_fd, buf, sizeof(buf)) > 0) {
                    if (write(client_fd, buf, n) < 0) {
                        perror("write failed.");
                    } else if (n == 0 || errno == ECONNRESET) {
                        close(client_fd);
                        continue;
                    } else {
                        write(client_fd, buf, n);
                    }
                }
            }
        }
    }
}

int create_sock() {
    int sock_fd = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(3000);
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    bind(sock_fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr));

    listen(sock_fd, 1024);

    printf("listening on port 3000.\n");

    return sock_fd;
}

void make_nonblocking(int fd) {
    fcntl(fd, F_SETFL, O_NONBLOCK);
}