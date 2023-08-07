#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <poll.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <errno.h>
#include <fcntl.h>

int create_sock();

void make_nonblocking(int fd);

int main(int argc, char *argv[]) {
    int read_num, conn_fd, n;
    char buf[1024];
    struct sockaddr_in client_addr;

    int sock_fd = create_sock();

    make_nonblocking(sock_fd);

    struct pollfd fds[1024];
    fds[0].fd = sock_fd;
    fds[0].events = POLLRDNORM;

    int i;
    for (i = 1; i < 1024; i++) {
        fds[i].fd = -1;
    }

    for (;;) {
        if ((read_num = poll(fds, 1024, -1)) < 0) {
            perror ("poll failed.");
        }

        if (fds[0].revents & POLLRDNORM) {
            socklen_t cli_len = sizeof((client_addr));
            conn_fd = accept(sock_fd, (struct sockaddr *)&client_addr, &cli_len);

            printf("have a new connection: %d.\n", conn_fd);

            for (i = 1; i < 1024; i++) {
                if (fds[i].fd < 0) {
                    fds[i].fd = conn_fd;
                    fds[i].events = POLLRDNORM;
                    break;
                }
            }

            if (i == 1024) {
                perror("too many clients.");
            }

            if (--read_num <= 0) {
                continue;
            }
        }

        for (i = 1; i < 1024; i++) {
            int client_fd;
            if ((client_fd = fds[i].fd) < 0) {
                continue;
            }

            if (fds[i].revents & (POLLRDNORM | POLLERR)) {
                if ((n = read(client_fd, buf, sizeof(buf))) > 0) {
                    if (write(client_fd, buf, n) < 0) {
                        perror("write failed.");
                    }
                } else if (n == 0 || errno == ECONNRESET) {
                    close(client_fd);
                    fds[i].fd = -1;
                } else {
                    perror("read failed.");
                }
            }

            if (--read_num <= 0) {
                break;
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