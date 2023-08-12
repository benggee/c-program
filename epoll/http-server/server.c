#include <stdio.h>
#include <stdlib.h>
#include "tcp_server.h"
#include "accepter.h"

int main(int argc, char *argv[]) {
    if (argc != 2) {
        perror("usage: ./server <port>");
        exit(1);
    }

    int sock_fd = create_tcp_socket(atoi(argv[1]));

    printf("server started on port %s\n", argv[1]);

    accept_sock(sock_fd);
}

