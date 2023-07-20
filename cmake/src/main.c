#include "tcp_server.h"
#include "thread_pool.h"
#include "reader.h"
#include "http_response.h"

int main(int argc, char *argv[]) {
    if (argc != 2) {
        perror("usage: ./server <port>");
        exit(1);
    }

    int sock_fd = create_tcp_socket(atoi(argv[1]));

    printf("server started on port %s\n", argv[1]);

    accepter_thread((void *)&sock_fd);
}