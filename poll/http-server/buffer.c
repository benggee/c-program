#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include "buffer.h"


struct buffer *new_buffer() {
    struct buffer *buf = malloc(sizeof(struct buffer));
    if (!buf) {
        return NULL;
    }

    buf->data = malloc(BUFFER_SIZE);
    memset(buf->data, 0, BUFFER_SIZE);
    buf->size = BUFFER_SIZE;
    buf->r_idx = 0;
    buf->w_idx = 0;

    return buf;
}

void buffer_free(struct buffer *buf) {
    free(buf->data);
    free(buf);
}

char buffer_read_char(struct buffer *buf) {
    if (buf->size - buf->r_idx == buf->w_idx) {
        return '\0';
    }

    char c = buf->data[buf->r_idx];
    if (buf->r_idx + 1 < buf->size) {
        buf->r_idx++;
    } else {
        buf->r_idx = 0;
    }

    return c;
}

int buffer_append(struct buffer *buf, char *data, size_t len) {
    if (buf->size - buf->r_idx == buf->w_idx) {
        return 0;
    }

    size_t write_size = 0;
    size_t write_total = 0;

    while(1) {
        if (buf->size - buf->r_idx == buf->w_idx || write_total == len) {
            break;
        }

        if (buf->r_idx > buf->w_idx) {
            write_size = buf->r_idx - buf->w_idx;
        } else if (buf->size - buf->w_idx < len) {
            write_size = buf->size - buf->w_idx;
        } else {
            write_size = len;
        }

        memcpy(buf->data + buf->w_idx, data, write_size);

        if (buf->w_idx + write_size < buf->size) {
            buf->w_idx += write_size;
        } else {
            buf->w_idx = 0;
        }
        write_total += write_size;
    }

    return write_total;
}

// if return is -2, it means that the buffer is full
int buffer_read_from_socket(struct buffer *buf, int sock_fd) {
    if (buf->size - buf->r_idx == buf->w_idx) {
        return -2;
    }

    int read_size = 0;
    if (buf->r_idx > buf->w_idx) {
        read_size = buf->r_idx - buf->w_idx;
    } else if (buf->size - buf->w_idx < BUFFER_SIZE) {
        read_size = buf->size - buf->w_idx;
    } else {
        read_size = BUFFER_SIZE;
    }

    int ret = recv(sock_fd, buf->data + buf->w_idx, read_size, 0);
    if (ret > 0) {
        if (buf->w_idx + ret < buf->size) {
            buf->w_idx += ret;
        } else {
            buf->w_idx = 0;
        }
    }

    return ret;
}

// if return is -2, it means that the buffer is empty
int buffer_write_to_socket(struct buffer *buf, int sock_fd) {
    if (buf->size - buf->r_idx == buf->w_idx) {
        return -2;
    }

    int write_size = 0;
    int write_total = 0;
    while(1) {
        if (buf->size - buf->r_idx == buf->w_idx) {
            break;
        }

        write_size = buf->size - buf->r_idx;
        if (buf->r_idx < buf->w_idx) {
            write_size = buf->w_idx - buf->r_idx;
        }

        int ret = send(sock_fd, buf->data + buf->r_idx, write_size, 0);
        if (ret <= 0) {
            return ret;
        }
        write_total += ret;

        if (buf->r_idx < buf->size) {
            buf->r_idx += ret;
        } else {
            buf->r_idx = 0;
        }
    }

    return write_total;
}