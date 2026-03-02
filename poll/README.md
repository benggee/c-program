# Poll 模块 - I/O 多路复用改进实现

## 概述

本模块实现了基于 `poll()` 的 I/O 多路复用服务器。Poll 是 select 的改进版本，解决了 select 的 FD 数量限制问题，在处理大量连接时更加灵活。

## Poll vs Select

| 特性 | Select | Poll |
|------|--------|------|
| **跨平台** | 所有 UNIX 系统 | 所有 UNIX 系统 |
| **FD 数量限制** | 有 (通常 1024) | 无 |
| **参数传递** | 每次需要重新设置 FD_SET | 直接传递数组 |
| **效率** | 需要线性扫描 FD_SET | 需要线性扫描数组 |

## Poll 系统调用

### 函数原型

```c
#include <poll.h>

int poll(struct pollfd *fds, nfds_t nfds, int timeout);
```

### 参数说明

| 参数 | 说明 |
|------|------|
| `fds` | pollfd 结构体数组 |
| `nfds` | 数组元素个数 |
| `timeout` | 超时时间（毫秒），-1 表示永久阻塞 |

### 返回值

| 返回值 | 含义 |
|--------|------|
| > 0 | 就绪 FD 的数量 |
| 0 | 超时 |
| -1 | 出错 |

## struct pollfd 结构

```c
struct pollfd {
    int   fd;         // 文件描述符
    short events;     // 监控的事件
    short revents;    // 实际发生的事件
};
```

### 常用 events 类型

| 事件 | 说明 |
|------|------|
| `POLLIN` | 有数据可读 |
| `POLLRDNORM` | 普通数据可读 |
| `POLLRDBAND` | 优先级带数据可读 |
| `POLLPRI` | 紧急数据可读 |
| `POLLOUT` | 写操作不会阻塞 |
| `POLLERR` | 发生错误 |
| `POLLHUP` | 连接挂断 |
| `POLLNVAL` | 无效请求 |

## 代码实现解析

### 初始化 pollfd 数组

```c
struct pollfd fds[1024];
fds[0].fd = sock_fd;           // 第 0 个位置放监听 socket
fds[0].events = POLLRDNORM;     // 监控可读事件

// 其他位置初始化为 -1（表示未使用）
for (i = 1; i < 1024; i++) {
    fds[i].fd = -1;
}
```

### 主循环

```c
for (;;) {
    // 等待任一 FD 就绪
    if ((read_num = poll(fds, 1024, -1)) < 0) {
        perror("poll failed.");
    }

    // 处理就绪的 FD
    ...
}
```

### 处理新连接

```c
if (fds[0].revents & POLLRDNORM) {
    conn_fd = accept(sock_fd, ...);

    printf("have a new connection: %d.\n", conn_fd);

    // 找到一个空闲位置
    for (i = 1; i < 1024; i++) {
        if (fds[i].fd < 0) {
            fds[i].fd = conn_fd;
            fds[i].events = POLLRDNORM;
            break;
        }
    }

    if (i == 1024) {
        perror("too many clients.");
    }

    if (--read_num <= 0) {
        continue;  // 没有其他就绪的 FD 了
    }
}
```

### 处理客户端数据

```c
for (i = 1; i < 1024; i++) {
    int client_fd = fds[i].fd;
    if (client_fd < 0) {
        continue;  // 跳过未使用的位置
    }

    if (fds[i].revents & (POLLRDNORM | POLLERR)) {
        if ((n = read(client_fd, buf, sizeof(buf))) > 0) {
            // 回显数据
            write(client_fd, buf, n);
        } else if (n == 0 || errno == ECONNRESET) {
            // 客户端断开连接
            close(client_fd);
            fds[i].fd = -1;  // 标记为未使用
        } else {
            perror("read failed.");
        }
    }

    if (--read_num <= 0) {
        break;  // 所有就绪的 FD 都已处理
    }
}
```

## Poll 的优势

### 1. 无 FD 数量限制

Select 受限于 FD_SETSIZE（通常 1024），而 poll 理论上没有限制：

```c
// select
fd_set read_fds;  // 最多 1024 个 FD

// poll
struct pollfd fds[10000];  // 可以监控更多
```

### 2. 更清晰的接口

```c
// select - 每次需要重新设置 FD_SET
fd_set read_fds;
FD_ZERO(&read_fds);
FD_SET(fd1, &read_fds);
FD_SET(fd2, &read_fds);
select(max_fd + 1, &read_fds, ...);

// poll - 直接使用数组
struct pollfd fds[] = {
    {fd1, POLLIN, 0},
    {fd2, POLLIN, 0}
};
poll(fds, 2, -1);
```

### 3. 更好的错误处理

```c
if (fds[i].revents & POLLERR) {
    // 处理错误
}
if (fds[i].revents & POLLHUP) {
    // 处理连接挂断
}
```

## Poll 的缺点

| 缺点 | 说明 |
|------|------|
| **仍需线性扫描** | 每次仍需遍历整个数组 |
| **数组管理** | 需要手动维护可用位置 |
| **性能下降** | 连接数增加时性能下降 |

## 性能对比

假设监控 n 个 FD：

| 操作 | Select | Poll |
|------|--------|------|
| 系统调用 | O(n) | O(n) |
| 用户态检查 | O(n) | O(n) |
| FD 限制 | 1024 | 无 |

## 编译和运行

```bash
# 编译
gcc -o server poll.c

# 运行服务器
./server
# listening on port 3000

# 测试连接（另一个终端）
telnet localhost 3000
# 或
nc localhost 3000
```

## 测试示例

### 单连接测试

```bash
$ nc localhost 3000
hello
hello              # 服务器回显
```

### 多连接测试

```bash
# 终端 1
$ nc localhost 3000

# 终端 2
$ nc localhost 3000

# 终端 3
$ nc localhost 3000
```

## 连接管理

当前实现使用固定大小数组管理连接：

```c
struct pollfd fds[1024];
```

### 查找空闲位置

```c
for (i = 1; i < 1024; i++) {
    if (fds[i].fd < 0) {
        // 找到空闲位置
        fds[i].fd = conn_fd;
        fds[i].events = POLLRDNORM;
        break;
    }
}
```

### 标记连接为空闲

```c
close(client_fd);
fds[i].fd = -1;  // -1 表示未使用
```

## 注意事项

1. **数组大小管理**
   ```c
   // 当前实现使用固定大小数组
   struct pollfd fds[1024];

   // 生产环境建议使用动态数组或链表
   struct pollfd *fds = malloc(max_clients * sizeof(struct pollfd));
   ```

2. **处理 -1 FD**
   ```c
   if (fds[i].fd < 0) {
       continue;  // 跳过未使用的位置
   }
   ```

3. **提前退出循环**
   ```c
   if (--read_num <= 0) {
       break;  // 所有就绪的 FD 都已处理
   }
   ```

## 适用场景

Poll 适用于以下场景：

1. **中等规模连接**：连接数 1000 - 10000
2. **跨平台需求**：需要支持多种 UNIX 系统
3. **超过 select 限制**：需要监控 > 1024 个 FD

## 不适用场景

1. **大规模连接**：连接数 > 10000，考虑使用 epoll (Linux)
2. **高性能要求**：poll 的线性扫描开销较大

## 下一步学习

- **epoll 模块**：Linux 上性能最优的解决方案
- **Reactor 模式**：事件驱动架构设计
- **HTTP 服务器**：实际应用场景

## HTTP 服务器版本

本目录还包含基于 poll 的 HTTP 服务器实现：

```bash
cd http-server
mkdir -p build && cd build
cmake ..
make
./http_server 8080
```

## 参考资料

- [Linux poll 手册](https://man7.org/linux/man-pages/man2/poll.2.html)
- [The C10K problem](http://www.kegel.com/c10k.html)
- [I/O 多路复用技术对比](https://colobu.com/2019/03/12/comparing-io-multiplex/)
