# Epoll 模块 - Linux 高性能 I/O 多路复用

## 概述

本模块实现了基于 `epoll` 的 I/O 多路复用服务器。Epoll 是 Linux 特有的高性能 I/O 事件通知机制，专为处理大量并发连接而设计，是构建高性能网络服务器的核心技术。

## 三种 I/O 多路复用技术对比

| 特性 | Select | Poll | Epoll |
|------|--------|------|-------|
| **跨平台** | 所有 UNIX | 所有 UNIX | **Linux 专属** |
| **FD 数量限制** | 有 (1024) | 无 | 无 |
| **时间复杂度** | O(n) | O(n) | **O(1)** |
| **最大连接数** | 受限 | 受内存限制 | **受内存限制** |
| **实现方式** | 遍历所有 FD | 遍历所有 FD | **回调 + 就绪列表** |

## 为什么 Epoll 更快

### Select/Poll 的工作方式

```
每次调用都需要遍历所有 FD：

[FD1, FD2, FD3, ..., FD1000]
  ↓    ↓    ↓           ↓
 检查 检查 检查  ...    检查
```

时间复杂度：O(n)，其中 n 是监控的 FD 总数

### Epoll 的工作方式

```
1. 注册 FD 时添加回调函数
2. FD 就绪时调用回调函数，将 FD 加入就绪列表
3. epoll_wait 只需返回就绪列表

[就绪列表] → [FD5, FD12, FD88]
           ↓
        只处理就绪的
```

时间复杂度：O(1)，只处理就绪的 FD

## Epoll 的三个核心函数

### 1. epoll_create1 - 创建 epoll 实例

```c
#include <sys/epoll.h>

int epoll_create1(int flags);
```

**功能**：创建一个 epoll 实例

**参数**：
- `flags`：通常设为 0，或 `EPOLL_CLOEXEC`

**返回值**：
- 成功：返回 epoll 文件描述符
- 失败：返回 -1

**示例**：
```c
int efd = epoll_create1(0);
if (efd == -1) {
    perror("epoll_create1 failed.");
    exit(1);
}
```

### 2. epoll_ctl - 管理 epoll 事件

```c
int epoll_ctl(int epfd, int op, int fd, struct epoll_event *event);
```

**功能**：添加、修改或删除要监控的文件描述符

**参数**：
- `epfd`：epoll 文件描述符
- `op`：操作类型
- `fd`：要监控的文件描述符
- `event`：指向 epoll_event 结构体的指针

**操作类型 (op)**：

| 操作 | 说明 |
|------|------|
| `EPOLL_CTL_ADD` | 注册新的 FD 到 epoll 实例 |
| `EPOLL_CTL_MOD` | 修改已注册 FD 的监控事件 |
| `EPOLL_CTL_DEL` | 从 epoll 实例中删除 FD |

**示例**：
```c
struct epoll_event event;
event.data.fd = sock_fd;
event.events = EPOLLIN | EPOLLET;  // 监控可读事件，边缘触发

if (epoll_ctl(efd, EPOLL_CTL_ADD, sock_fd, &event) == -1) {
    perror("epoll_ctl failed.");
    exit(1);
}
```

### 3. epoll_wait - 等待事件

```c
int epoll_wait(int epfd, struct epoll_event *events,
               int maxevents, int timeout);
```

**功能**：等待事件发生

**参数**：
- `epfd`：epoll 文件描述符
- `events`：输出参数，存储发生的事件
- `maxevents`：最多返回多少个事件
- `timeout`：超时时间（毫秒），-1 表示永久阻塞

**返回值**：
- 成功：返回就绪的 FD 数量
- 超时：返回 0
- 失败：返回 -1

**示例**：
```c
#define MAX_EVENTS 1024
struct epoll_event events[MAX_EVENTS];

int nfds = epoll_wait(efd, events, MAX_EVENTS, -1);
if (nfds == -1) {
    perror("epoll_wait failed.");
    exit(1);
}

// 处理所有就绪的事件
for (int i = 0; i < nfds; i++) {
    if (events[i].events & EPOLLIN) {
        // 处理可读事件
    }
}
```

## struct epoll_event 结构

```c
struct epoll_event {
    uint32_t     events;    // epoll 事件
    epoll_data_t data;      // 用户数据
};

typedef union epoll_data {
    int     fd;             // 文件描述符
    void    *ptr;           // 用户数据指针
    uint32_t u32;           // 32 位整数
    uint64_t u64;           // 64 位整数
} epoll_data_t;
```

## 事件类型

| 事件 | 说明 |
|------|------|
| `EPOLLIN` | 有数据可读 |
| `EPOLLOUT` | 可以写入数据（发送缓冲区未满） |
| `EPOLLRDHUP` | 对方关闭连接或关闭写端 |
| `EPOLLPRI` | 有紧急数据可读 |
| `EPOLLERR` | 发生错误 |
| `EPOLLHUP` | 连接挂断 |
| `EPOLLET` | **边缘触发 (Edge Triggered)** |
| `EPOLLONESHOT` | 只监控一次，之后自动删除 |

## 两种触发模式

### 水平触发 (Level-Triggered, LT)

**默认模式**

```c
event.events = EPOLLIN;  // 默认为水平触发
```

**特点**：
- 只要缓冲区有数据，就会一直通知
- 编程简单，不容易丢失事件
- 可能造成多次唤醒

**工作流程**：
```
FD 有数据 → 通知 → 未读完 → 下次 wait 继续通知 → 读完
```

### 边缘触发 (Edge-Triggered, ET)

**高性能模式**

```c
event.events = EPOLLIN | EPOLLET;  // 边缘触发
```

**特点**：
- 只在状态变化时通知一次
- 必须一次性读完所有数据
- 需要配合非阻塞 I/O
- 效率更高，减少系统调用

**工作流程**：
```
无数据 → 有数据 → 通知 → 必须读完 → 不再通知直到下一次数据到来
```

**ET 模式注意事项**：
1. 必须使用非阻塞 Socket
2. 必须循环读取直到 EAGAIN
3. 必须处理部分写的情况

## 代码实现解析

### 初始化 epoll

```c
#define MAX_EVENTS 1024

int efd = epoll_create1(0);
if (efd == -1) {
    perror("epoll_create1 failed.");
    exit(1);
}

struct epoll_event event;
event.data.fd = sock_fd;
event.events = EPOLLIN | EPOLLET;  // 边缘触发

if (epoll_ctl(efd, EPOLL_CTL_ADD, sock_fd, &event) == -1) {
    perror("epoll_ctl failed.");
    exit(1);
}

struct epoll_event *events = calloc(MAX_EVENTS, sizeof(event));
```

### 主循环

```c
for (;;) {
    // 等待事件发生
    int nfds = epoll_wait(efd, events, MAX_EVENTS, -1);

    // 处理所有就绪的事件
    for (int i = 0; i < nfds; i++) {
        ...
    }
}
```

### 处理错误和连接关闭

```c
if ((events[i].events & EPOLLERR) ||
    (events[i].events & EPOLLHUP) ||
    (!(events[i].events & EPOLLIN)))
{
    // 发生错误或连接关闭
    perror("epoll error.");
    close(events[i].data.fd);
    continue;
}
```

### 处理新连接

```c
if (sock_fd == events[i].data.fd) {
    // 监听 socket 就绪，有新连接
    socklen_t cli_len = sizeof(client_addr);
    conn_fd = accept(sock_fd, (struct sockaddr *)&client_addr, &cli_len);
    if (conn_fd == -1) {
        perror("accept error.");
        continue;
    }

    // 将新连接加入 epoll 监控
    event.data.fd = conn_fd;
    event.events = EPOLLIN | EPOLLET;
    if (epoll_ctl(efd, EPOLL_CTL_ADD, conn_fd, &event) == -1) {
        perror("epoll_ctl error.");
    }
    continue;
}
```

### 处理客户端数据

```c
int client_fd = events[i].data.fd;

if ((n = read(client_fd, buf, sizeof(buf))) > 0) {
    // 读取数据并回显
    write(client_fd, buf, n);
} else if (n == 0 || errno == ECONNRESET) {
    // 客户端关闭连接
    close(client_fd);
}
```

## ET 模式的正确实现

边缘触发模式下，必须循环读取直到 EAGAIN：

```c
while (1) {
    ssize_t count = read(fd, buf, sizeof(buf));
    if (count == -1) {
        if (errno != EAGAIN) {
            // 真正的错误
            perror("read");
        }
        break;  // 读完了
    } else if (count == 0) {
        // 连接关闭
        close(fd);
        break;
    }

    // 处理数据
    process_data(buf, count);
}
```

## Epoll 的优势

| 优势 | 说明 |
|------|------|
| **O(1) 时间复杂度** | 只处理就绪的 FD |
| **支持大规模连接** | 可轻松处理 10 万+ 连接 |
| **边缘触发** | 减少系统调用次数 |
| **无需遍历** | 直接返回就绪的 FD |

## 性能对比

假设监控 10000 个 FD，其中只有 100 个就绪：

| 技术 | 需要检查的 FD | 系统调用次数 |
|------|---------------|--------------|
| select | 10000 | 1 |
| poll | 10000 | 1 |
| **epoll** | **100** | **1** |

## 编译和运行

```bash
# 编译
gcc -o server epoll.c

# 运行服务器
./server
# listening on port 3000

# 测试连接（另一个终端）
telnet localhost 3000
```

## 压力测试

```bash
# 使用 Apache Bench 进行压力测试
ab -n 100000 -c 1000 http://localhost:3000/

# 使用 wrk
wrk -t12 -c400 -d30s http://localhost:3000/
```

## 适用场景

Epoll 适用于以下场景：

1. **大规模连接**：连接数 > 10000
2. **高性能要求**：需要最大化性能
3. **Linux 系统**：仅限 Linux 平台
4. **事件驱动架构**：配合 Reactor 模式

## 不适用场景

1. **跨平台需求**：需要支持 Windows/macOS
2. **小规模连接**：连接数 < 1000，poll 足够

## 实际应用

使用 epoll 的著名项目：

| 项目 | 说明 |
|------|------|
| **Nginx** | 高性能 Web 服务器 |
| **Redis** | 内存数据库 |
| **Node.js** | JavaScript 运行时 (libuv) |
| **Netty** | Java 网络框架 (Linux 版) |

## HTTP 服务器版本

本目录还包含基于 epoll 的 HTTP 服务器实现：

```bash
cd http-server
mkdir -p build && cd build
cmake ..
make
./http_server 8080

# 测试
curl http://localhost:8080
```

## 注意事项

1. **边缘触发必须循环读取**
   ```c
   while (read(fd, buf, sizeof(buf)) > 0) {
       // 处理数据
   }
   ```

2. **记得设置非阻塞**
   ```c
   fcntl(fd, F_SETFL, O_NONBLOCK);
   ```

3. **记得删除已关闭的 FD**
   ```c
   close(fd);
   epoll_ctl(efd, EPOLL_CTL_DEL, fd, NULL);
   ```

## 下一步学习

- **HTTP 服务器**：实际应用场景
- **Reactor 模式**：事件驱动架构设计
- **线程池 + epoll**：进一步优化性能

## 参考资料

- [Linux epoll 手册](https://man7.org/linux/man-pages/man7/epoll.7.html)
- [The C10K problem](http://www.kegel.com/c10k.html)
- [Epoll 原理详解](https://github.com/netty/netty/wiki/NATIVE-TRANSPORTS)
