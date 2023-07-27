
#define BUFFER_SIZE 4096

struct buffer {
    char *data; // bufer data
    int r_idx;  // read index
    int w_idx;  // write index
    int size;   // buffer size
};

struct buffer *new_buffer();

void free_buffer(struct buffer *buf);

void buffer_allocation(struct buffer *buf, void *data, int size);

int buffer_read_from_socket(struct buffer *buf, int sock_fd);