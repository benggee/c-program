#include <sys/socket.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "handle_msg.h"

void do_msg(struct sock_buffer *s_buffer, int sock_fd) {
    if (s_buffer == NULL) {
        perror("invalid sock_buffer");
        return;
    }

    printf("do-msg: %d\n", sock_fd);

    int len = 0; // 0-表示还在读长度 >0表示已经读到长度
    char len_char[5];   // 消息长度
    int len_idx = 0;   // 长度读到哪里了

    int msg_len_idx = 0;

    char msg[9999];

    struct buffer *r_buffer = s_buffer->r_buffer;
    while(1) {
        int ret = buffer_read_from_socket(r_buffer, sock_fd);
        if (ret == -1) {
            if (errno == EAGAIN) {
                continue;
            }
            perror("read error");
            break;
        }
        if (ret == 0) {
            close(sock_fd);
            break;
        }


        while(ret > 0) {
            while (len == 0 && len_idx < MSG_LEN) {
                char c = buffer_read_char(s_buffer->r_buffer);
                if (c != '\0') {
                    len_char[len_idx++] = c;
                    if (len_idx == MSG_LEN) {
                        memset(msg, 0, sizeof(msg));
                        len_char[len_idx] = '\0';
                        len = atoi(len_char);
                        len_idx = 0;
                    }
                } else {
                    ret = 0;
                    break;
                }
            }

            while (1) {
                char c = buffer_read_char(s_buffer->r_buffer);
                if (c != '\0') {
                    msg[msg_len_idx++] = c;
                    if (msg_len_idx == len) {
                        msg[msg_len_idx] = '\0';
                        printf("msg: %s\n", msg);
                        msg_len_idx = 0;
                        len = 0;

                        //buffer_append(s_buffer->w_buffer, msg, strlen(msg));
                        //buffer_write_to_socket(s_buffer->w_buffer, sock_fd);
                        break;
                    }
                } else {
                    ret = 0;
                    break;
                }
            }
        }
    }
}