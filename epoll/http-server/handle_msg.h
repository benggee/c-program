#ifndef __HANDLE_MSG_H__
#define __HANDLE_MSG_H__

#include "buffer_map.h"
#include "buffer.h"


#define MSG_LEN  4

void do_msg(struct sock_buffer *s_buffer, int sock_fd);

#endif