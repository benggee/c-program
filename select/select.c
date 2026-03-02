/**
 * @file select.c
 * @brief 基于 select() 的 I/O 多路复用 TCP 服务器
 *
 * Select 允许单个进程/线程同时监控多个文件描述符 (FD)，
 * 当其中任何一个或多个 FD 就绪时，select 会返回，
 * 然后程序可以对就绪的 FD 进行相应的 I/O 操作。
 *
 * 工作流程：
 * 1. 初始化 FD_SET，添加要监控的 FD
 * 2. 调用 select() 等待任一 FD 就绪
 * 3. 遍历 FD_SET，检查哪些 FD 已就绪
 * 4. 处理就绪的 FD (读/写/错误)
 * 5. 返回步骤 2
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>

/**
 * @brief 创建并配置监听 Socket
 * @return 成功返回 Socket FD，失败返回 -1
 */
int create_sock();

/**
 * @brief 将 Socket 设置为非阻塞模式
 * @param sock_fd Socket 文件描述符
 */
void make_nonblocking(int sock_fd);


int main(int argc, char *argv[]) {
    struct sockaddr_in client_addr;
    int conn_fd, serv_fd, max_fd;
    char recv_buf[1024], send_buf[1024];

    serv_fd = create_sock();
    make_nonblocking(serv_fd);

    fd_set read_mask;
    fd_set all_reads;

    FD_ZERO(&all_reads);
    FD_SET(0, &all_reads);
    FD_SET(serv_fd, &all_reads);

    memset(&client_addr, 0, sizeof(client_addr));
    socklen_t cli_len = sizeof(client_addr);
    max_fd = serv_fd;

    while(1) {
        read_mask = all_reads;

        int rc = select(max_fd + 1, &read_mask, NULL, NULL, NULL);
        if (rc < 0) {
            perror("select failed.");
            exit(1);
        }

        if (FD_ISSET(serv_fd, &read_mask)) {
            memset(&client_addr, 0, sizeof(client_addr));
            conn_fd = accept(serv_fd, (struct sockaddr *)&client_addr, &cli_len);
            if (conn_fd < 0) {
                perror("accept failed.");
                continue;
            }

            if (conn_fd > max_fd) {
                max_fd = conn_fd;
            }
            FD_SET(conn_fd, &all_reads);

            printf("have a new connection: %d.\n", conn_fd);
        }

        if (FD_ISSET(STDIN_FILENO, &read_mask)) {
            memset(send_buf, 0, sizeof(send_buf));
            if (conn_fd <= 0) {
                fgets(send_buf, sizeof(send_buf), stdin);
                printf("please wait for a connection.\n");
                continue;
            }

            if (fgets(send_buf, sizeof(send_buf), stdin) != NULL) {
                int i = strlen(send_buf);
                if (send_buf[i - 1] == '\n') {
                    send_buf[i - 1] = '\0';
                }

                printf("now send: %s\n", send_buf);

                size_t rt = write(conn_fd, send_buf, strlen(send_buf));
                if (rt < 0) {
                    perror("write failed.");
                }
                printf("send bytes: %zu\n", rt);
            }
        }

        if (FD_ISSET(conn_fd, &read_mask)) {
            memset(recv_buf, 0, sizeof(recv_buf));
            size_t rt = read(conn_fd, recv_buf, sizeof(recv_buf));
            if (rt < 0) {
                printf("read failed. conn-fd: %d\n", conn_fd);
                continue;
            }
            if (rt == 0) {
                printf("client closed, conn-fd: %d\n", conn_fd);
                FD_CLR(conn_fd, &all_reads);
                close(conn_fd);
                continue;
            }

            printf("from client msg: %s\n", recv_buf);

            send(conn_fd, recv_buf, strlen(recv_buf), 0);
        }
    }

}

void make_nonblocking(int sock_fd) {
    fcntl(sock_fd, F_SETFL, O_NONBLOCK);
}

int create_sock() {
    int sock_fd = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in serv_addr;

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(3000);
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    int on = 1;
    setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));

    if (bind(sock_fd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        perror("bind failed.");
        return -1;
    }

    if (listen(sock_fd, 1024) < 0) {
        perror("listen failed.");
        return -1;
    }

    return sock_fd;
}