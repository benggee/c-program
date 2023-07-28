#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include "buffer.h"

int create_sock();

void request_with_buffer(int sock_fd);

void response_with_buffer(int sock_fd);

void on_http_request(int sock_fd);

int main(int argc, char *argv[]) {

    int sock_fd = create_sock();
    struct sockaddr_in client_addr;

    while(1) {
        socklen_t cli_len = sizeof(client_addr);
        memset(&client_addr, 0, sizeof(client_addr));

        int fd = accept(sock_fd, (struct sockaddr *)&client_addr, &cli_len);
        if (fd < 0) {
            perror("accept error");
            exit(1);
        }

        on_http_request(fd);
    }
}

void on_http_request(int sock_fd) {
    request_with_buffer(sock_fd);

    response_with_buffer(sock_fd);

    close(sock_fd);
}

void request_with_buffer(int sock_fd) {
    struct buffer *buf = new_buffer();

    buffer_read_from_socket(buf, sock_fd);

    printf("%s\n", buf->data);

    buffer_free(buf);
}

void response_with_buffer(int sock_fd) {
    struct buffer *buf = new_buffer();

    buffer_append(buf, "HTTP/1.1 200 OK\r\n", strlen("HTTP/1.1 200 OK\r\n"));
    buffer_append(buf, "Host: 127.0.0.1:3000\r\n", strlen("Host: 127.0.0.1:3000\r\n"));
    buffer_append(buf, "Content-Type: text/html; charset=UTF-8\r\n", strlen("Content-Type: text/html; charset=UTF-8\r\n"));
    buffer_append(buf, "\r\n", strlen("\r\n"));

    char *body = "<html><head><title>hello</title></head><body><h1>This is My HTTP Server</h1></body></html>";
    buffer_append(buf, body, strlen(body));

    buffer_write_to_socket(buf, sock_fd);

    buffer_free(buf);
}

int create_sock() {
    int sock_fd;
    struct sockaddr_in serv_addr;

    sock_fd = socket(AF_INET, SOCK_STREAM, 0);

    memset(&serv_addr, 0 , sizeof(serv_addr));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(3000);
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    int on = 1;
    setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));

    bind(sock_fd, (struct sockaddr *) &serv_addr, sizeof(serv_addr));

    listen(sock_fd, 1024);

    printf("Server is running at: %d\n", 3000);

    return sock_fd;
}