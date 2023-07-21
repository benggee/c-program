#include "http_response.h"

void write_response(int fd) {
    send(fd, "HTTP/1.1 200 OK\r\n", strlen("HTTP/1.1 200 OK\r\n"), 0);
    send(fd, "Host: 127.0.0.1:3000\r\n", strlen("Host: 127.0.0.1:3000\r\n"), 0);
    send(fd, "Content-Type: text/html; charset=UTF-8\r\n", strlen("Content-Type: text/html; charset=UTF-8\r\n"), 0);
    send(fd, "\r\n", strlen("\r\n"), 0);

    char *body = "<html><head><title>hello</title></head><body><h1>This is My HTTP Server</h1></body></html>";
    send(fd, body, strlen(body), 0);

    close(fd);
}