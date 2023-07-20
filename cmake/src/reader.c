#include "reader.h"

size_t read_line(int fd, char *buf, size_t size) {
    size_t i = 0;
    ssize_t  n;
    char c = '\0';

    while((i < size) && (c != '\n')) {
        n = recv(fd, &c, 1, 0);
        if (n > 0) {
            if (c == '\r') {
                n = recv(fd, &c, 1, MSG_PEEK);
                if ((n > 0) && (c == '\n')) {
                    recv(fd, &c, 1, 0);
                } else {
                    c = '\n';
                }
            }
            buf[i] = c;
            i++;
        } else {
            c = '\n';
        }
    }
    buf[i] = '\0';
    return i;
}