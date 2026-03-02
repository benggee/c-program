# Non-Blocking Socket + 线程池模块

## 概述

本模块实现了一个高性能的 TCP 服务器，结合了**非阻塞 Socket**、**循环缓冲区**和**线程池**三种技术。这是一个综合性很强的项目，展示了如何在实际生产环境中构建高性能网络服务器。

## 核心技术

### 1. 非阻塞 Socket

将 Socket 设置为非阻塞模式后，I/O 操作不会阻塞当前线程：

```c
void make_nonblocking(int fd) {
    fcntl(fd, F_SETFL, O_NONBLOCK);
}
```

**优势**：
- 读写操作立即返回
- 可以同时处理多个连接
- 配合线程池实现高并发

### 2. 循环缓冲区

每个 Socket 连接都有独立的读写缓冲区：

```c
struct sock_buffer {
    struct buffer *w_buffer;  // 写缓冲区
    struct buffer *r_buffer;  // 读缓冲区
};
```

### 3. 线程池架构

```
┌─────────────────────────────────────────────────────┐
│                    主线程                           │
│             (accepter_thread)                       │
│        接受新连接，分配给 Worker 线程                │
└─────────────────────────────────────────────────────┘
                    │
                    │ 分发任务
                    ▼
┌─────────────────────────────────────────────────────┐
│                  Worker 线程池                       │
│  ┌─────┐  ┌─────┐  ┌─────┐  ┌─────┐  ┌─────┐      │
│  │ W1  │  │ W2  │  │ W3  │  │ W4  │  │ W5  │      │
│  └─────┘  └─────┘  └─────┘  └─────┘  └─────┘      │
│      等待任务，处理客户端请求                        │
└─────────────────────────────────────────────────────┘
```

## 项目结构

```
non-blocking-socket/
├── buffer.h/c          # 循环缓冲区实现
├── buffer_map.h/c      # Socket 到缓冲区的映射
├── thread_pool.h/c     # 线程池实现
├── tcp_server.h/c      # TCP 服务器基础
├── common.h            # 公共定义
├── server.c            # 服务器入口
└── CMakeLists.txt      # CMake 构建配置
```

## 核心数据结构

### 1. worker_thread_context

Worker 线程的上下文信息：

```c
struct worker_thread_context {
    int fd;                         // 当前处理的 FD
    pthread_t thread_id;            // 线程 ID
    pthread_mutex_t mutex;          // 互斥锁
    pthread_cond_t cond;            // 条件变量
    struct buffer_map *buffer_map;  // 缓冲区映射
};
```

### 2. buffer_map

管理所有 Socket 的缓冲区：

```c
struct buffer_map {
    struct sock_buffer **s_buffer;  // 动态数组
    int max_idx;                    // 最大索引
};

struct sock_buffer {
    struct buffer *w_buffer;  // 写缓冲区
    struct buffer *r_buffer;  // 读缓冲区
};
```

### 3. buffer

循环缓冲区实现：

```c
struct buffer {
    char *data;    // 数据
    int r_idx;     // 读指针
    int w_idx;     // 写指针
    int size;      // 大小
};
```

## 工作流程

### 1. 初始化阶段

```
主线程
  │
  ├─ 创建 TCP Socket
  │
  ├─ 创建 Buffer Map
  │
  └─ 启动 Worker 线程池
      │
      └─ 创建 N 个 Worker 线程
          │
          └─ 每个线程初始化互斥锁和条件变量
              │
              └─ 进入等待状态 (pthread_cond_wait)
```

### 2. 运行阶段

```
Accepter 线程                  Worker 线程
     │                            │
     │ accept()                   │
     ├───────────┐                │
     │           │                │
     ▼           │                │
 新连接         │                │
     │           │                │
     │ 设置非阻塞  │                │
     │           │                │
     │ 分配缓冲区  │                │
     │           │                │
     ├───────────┼───────────────>│
     │           │    pthread_cond_signal
     │           │                │
     │           │                │ pthread_cond_wait 返回
     │           │                │
     │           │                ▼
     │           │          处理客户端请求
     │           │                │
     │           │                │ 读取数据
     │           │                │ 处理业务
     │           │                │ 发送响应
     │           │                │
     │           │                │
     │           │                │ 重新进入等待
     │           │                ▼
     │           │          pthread_cond_wait
     │           │                │
     │           │                │
     ▼           ▼                ▼
 继续接受下一个连接
```

## 代码解析

### Accepter 线程 (thread_pool.c:104-157)

```c
void *accepter_thread(void *arg) {
    int fd = *(int *)arg;

    // 1. 创建 Buffer Map
    struct buffer_map *map = new_buffer_map();

    // 2. 创建 Worker 线程池
    struct worker_thread_context *ctx =
        malloc(WORKER_POOL_SIZE * sizeof(struct worker_thread_context));
    ctx->buffer_map = map;

    for (int i = 0; i < WORKER_POOL_SIZE; i++) {
        pthread_mutex_init(&ctx[i].mutex, NULL);
        pthread_cond_init(&ctx[i].cond, NULL);
        ctx[i].fd = -1;  // -1 表示空闲
        pthread_create(&ctx[i].thread_id, NULL, worker, &ctx[i]);
    }

    // 3. 接受连接并分发
    int idx = 0;
    while(1) {
        int conn_fd = accept(fd, ...);
        make_nonblocking(conn_fd);

        // 为新连接分配缓冲区
        allocation_buffer_map(map, conn_fd);

        // 轮询分配给 Worker 线程
        pthread_mutex_lock(&ctx[idx].mutex);
        ctx[idx].fd = conn_fd;
        pthread_mutex_unlock(&ctx[idx].mutex);
        pthread_cond_signal(&ctx[idx].cond);  // 唤醒 Worker

        if (++idx == WORKER_POOL_SIZE) {
            idx = 0;  // 循环分配
        }
    }
}
```

### Worker 线程 (thread_pool.c:83-102)

```c
void *worker(void *arg) {
    struct worker_thread_context *ctx = (struct worker_thread_context *)arg;

    while (1) {
        // 1. 等待任务
        pthread_mutex_lock(&ctx->mutex);
        pthread_cond_wait(&ctx->cond, &ctx->mutex);

        // 2. 检查 FD 是否有效
        if (ctx->fd == -1) {
            break;  // 退出信号
        }

        // 3. 获取对应的缓冲区
        struct sock_buffer *s_buffer = ctx->buffer_map->s_buffer[ctx->fd];

        // 4. 处理消息
        handle_msg(s_buffer, ctx->fd);

        pthread_mutex_unlock(&ctx->mutex);
    }
}
```

### 消息处理 (thread_pool.c:13-81)

```c
void handle_msg(struct sock_buffer *s_buffer, int sock_fd) {
    // 消息格式：4 字节长度 + 消息内容
    int len = 0;           // 消息长度
    char len_char[5];      // 长度字符串
    int len_idx = 0;       // 长度读到哪里
    int msg_len_idx = 0;   // 消息读到哪里
    char msg[9999];

    struct buffer *r_buffer = s_buffer->r_buffer;

    while(1) {
        // 从 Socket 读取数据到缓冲区
        int ret = buffer_read_from_socket(r_buffer, sock_fd);
        if (ret == -1) {
            if (errno == EAGAIN) {
                continue;  // 非阻塞，重试
            }
            break;
        }
        if (ret == 0) {
            close(sock_fd);  // 连接关闭
            break;
        }

        // 读取 4 字节长度
        while (len == 0 && len_idx < MSG_LEN) {
            char c = buffer_read_char(r_buffer);
            if (c != '\0') {
                len_char[len_idx++] = c;
                if (len_idx == MSG_LEN) {
                    len_char[len_idx] = '\0';
                    len = atoi(len_char);  // 解析长度
                    len_idx = 0;
                }
            } else {
                break;  // 数据不够
            }
        }

        // 读取消息内容
        while (len > 0) {
            char c = buffer_read_char(r_buffer);
            if (c != '\0') {
                msg[msg_len_idx++] = c;
                if (msg_len_idx == len) {
                    msg[msg_len_idx] = '\0';
                    printf("msg: %s\n", msg);

                    // 回显消息
                    buffer_append(s_buffer->w_buffer, msg, strlen(msg));
                    buffer_write_to_socket(s_buffer->w_buffer, sock_fd);

                    msg_len_idx = 0;
                    len = 0;
                    break;
                }
            } else {
                break;  // 数据不够
            }
        }
    }
}
```

## Buffer Map 动态扩容

`allocation_buffer_map` 实现了动态扩容：

```c
int allocation_buffer_map(struct buffer_map *map, int slot) {
    // 如果 slot 已在范围内，直接分配
    if (map->max_idx > slot) {
        map->s_buffer[slot] = new_sock_buffer();
        return 0;
    }

    // 否则扩容：每次扩容为 2 的幂次
    int max_idx = map->max_idx ? map->max_idx : 32;
    while (max_idx <= slot) {
        max_idx <<= 1;  // max_idx *= 2
    }

    // 重新分配内存
    struct sock_buffer **tmp =
        realloc(map->s_buffer, max_idx * sizeof(struct sock_buffer *));

    // 分配新的 sock_buffer
    tmp[slot] = new_sock_buffer();

    map->max_idx = max_idx;
    map->s_buffer = tmp;

    return 0;
}
```

**扩容策略**：
- 初始大小：32
- 扩容倍数：每次 x2
- 扩容序列：32 → 64 → 128 → 256 → ...

## 互斥锁和条件变量

### 为什么需要互斥锁？

保护共享数据 `ctx->fd`：

```c
pthread_mutex_lock(&ctx->mutex);
ctx->fd = conn_fd;        // 临界区
pthread_mutex_unlock(&ctx->mutex);
```

### 为什么需要条件变量？

实现生产者-消费者模式：

```c
// Worker 线程（消费者）
pthread_mutex_lock(&ctx->mutex);
pthread_cond_wait(&ctx->cond, &ctx->mutex);  // 等待任务
// ... 处理任务 ...
pthread_mutex_unlock(&ctx->mutex);

// Accepter 线程（生产者）
pthread_mutex_lock(&ctx->mutex);
ctx->fd = conn_fd;
pthread_mutex_unlock(&ctx->mutex);
pthread_cond_signal(&ctx->cond);  // 通知有新任务
```

## 编译和运行

```bash
# 创建构建目录
mkdir -p build && cd build

# 配置
cmake ..

# 编译
make

# 运行服务器
./server 8080
# server started on port 8080
```

## 测试客户端

可以使用 telnet 或 nc 测试：

```bash
# 连接服务器
nc localhost 8080

# 发送消息（格式：4 位长度 + 消息）
# 例如：0005hello
```

**注意**：消息格式为 4 位十进制长度 + 消息内容

```bash
# 发送 "hello"（长度 5）
echo -n "0005hello" | nc localhost 8080

# 发送 "world"（长度 5）
echo -n "0005world" | nc localhost 8080
```

## 性能优化建议

### 1. 增加 Worker 线程数

修改 `common.h`：

```c
#define WORKER_POOL_SIZE 8  // 根据 CPU 核心数调整
```

### 2. 调整缓冲区大小

修改 `buffer.h`：

```c
#define BUFFER_SIZE 131071  // 128KB
```

### 3. 使用更好的调度算法

当前使用简单的轮询，可以改为：

- 最少连接数优先
- CPU 亲和性绑定
- 工作窃取 (Work Stealing)

## 注意事项

1. **线程安全**
   - 每个连接同一时刻只能被一个 Worker 处理
   - Buffer Map 的访问需要加锁

2. **内存泄漏**
   - 连接关闭时记得释放缓冲区
   - 程序退出时记得销毁线程池

3. **错误处理**
   - 处理 EAGAIN（非阻塞）
   - 处理连接重置（ECONNRESET）

## 适用场景

1. **长连接服务器**：如聊天服务器
2. **消息推送**：实时消息推送系统
3. **游戏服务器**：多人在线游戏

## 与其他模块的关系

| 模块 | 特点 |
|------|------|
| **buffer** | 基础缓冲区实现 |
| **select/poll/epoll** | I/O 多路复用 |
| **non-blocking-socket** | 结合多种技术的高性能架构 |

## 下一步学习

- **epoll HTTP 服务器**：结合 epoll 的事件驱动
- **Reactor 模式**：更优雅的事件处理架构
- **Proactor 模式**：异步 I/O 模型

## 参考资料

- [POSIX 线程编程](https://computing.llnl.gov/tutorials/pthreads/)
- [非阻塞 I/O](https://www.keil.com/pack/doc/mw6/Network/html/group__nonblocking__io.html)
- [生产者-消费者问题](https://en.wikipedia.org/wiki/Producer%E2%80%93consumer_problem)
