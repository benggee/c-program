#ifndef __THREAD_POOL_H__
#define __THREAD_POOL_H__

#include "common.h"

void *worker(void *arg);

void *accepter_thread(void *arg);

#endif