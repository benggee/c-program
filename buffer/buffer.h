
#define BUFFER_SIZE 65535

struct buffer {
    char *data; // bufer data
    int r_idx;  // read index
    int w_idx;  // write index
    int size;   // buffer size
};

struct buffer *new_buffer();

void buffer_free(struct buffer *buf);

int buffer_append(struct buffer *buf, char *data, size_t len);

int buffer_read_from_socket(struct buffer *buf, int sock_fd);

int buffer_write_to_socket(struct buffer *buf, int sock_fd);