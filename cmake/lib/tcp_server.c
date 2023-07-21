#include "tcp_server.h"

int create_tcp_socket(int port) {
    int sock_fd;
    struct sockaddr_in serv_addr;

    sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    bzero(&serv_addr, sizeof(serv_addr));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    int on = 1;
    setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));

    bind(sock_fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr));

    listen(sock_fd, 1024);

    return sock_fd;
}