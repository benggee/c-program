# Buffer 模块 - 循环缓冲区实现

## 概述

本模块实现了一个高效的循环缓冲区（Circular Buffer），特别适用于网络编程中的数据收发场景。循环缓冲区通过固定大小的数组和两个指针（读指针和写指针）来实现数据的存储和读取，避免了频繁的内存分配和数据拷贝。

## 什么是循环缓冲区

循环缓冲区是一种先进先出（FIFO）的数据结构，具有以下特点：

1. **固定大小**：预分配固定大小的内存空间
2. **环形结构**：当读写指针到达数组末尾时，会自动回到开头
3. **双指针管理**：
   - `r_idx` (read index)：读指针，指向下一个要读取的数据位置
   - `w_idx` (write index)：写指针，指向下一个要写入的位置

## 为什么使用循环缓冲区

### 传统方式的问题

```c
// 传统方式：每次都分配和拷贝内存
char buf[1024];
recv(sockfd, buf, sizeof(buf), 0);
process(buf);  // 可能需要再次拷贝
```

### 循环缓冲区的优势

| 优势 | 说明 |
|------|------|
| **减少内存分配** | 预分配固定空间，避免频繁 malloc/free |
| **减少数据拷贝** | 数据直接写入缓冲区，处理时直接读取 |
| **支持分片处理** | 可以处理不完整的数据包 |
| **线程安全友好** | 配合锁机制，易于实现生产者-消费者模式 |

## 数据结构定义

```c
#define BUFFER_SIZE 65535  // 缓冲区大小 64KB

struct buffer {
    char *data;    // 缓冲区数据
    int r_idx;     // 读指针位置
    int w_idx;     // 写指针位置
    int size;      // 缓冲区总大小
};
```

### 缓冲区状态

```
空缓冲区：r_idx == w_idx

满缓冲区：size - r_idx == w_idx

数据存储在 [r_idx, w_idx) 区间
```

## API 接口

### 1. 创建缓冲区

```c
struct buffer *new_buffer();
```

**功能**：创建一个新的循环缓冲区

**返回值**：
- 成功：返回缓冲区指针
- 失败：返回 NULL

**示例**：
```c
struct buffer *buf = new_buffer();
if (buf == NULL) {
    perror("Failed to create buffer");
    exit(1);
}
```

### 2. 释放缓冲区

```c
void buffer_free(struct buffer *buf);
```

**功能**：释放缓冲区及其内部内存

**参数**：
- `buf`：要释放的缓冲区指针

**示例**：
```c
buffer_free(buf);
buf = NULL;
```

### 3. 追加数据

```c
int buffer_append(struct buffer *buf, char *data, size_t len);
```

**功能**：将数据追加到缓冲区

**参数**：
- `buf`：缓冲区指针
- `data`：要追加的数据
- `len`：数据长度

**返回值**：
- 成功：返回实际写入的字节数
- 缓冲区满：返回 0

**特点**：
- 自动处理环绕写入
- 不会覆盖未读数据

### 4. 从 Socket 读取数据

```c
int buffer_read_from_socket(struct buffer *buf, int sock_fd);
```

**功能**：从 Socket 读取数据并存入缓冲区

**参数**：
- `buf`：缓冲区指针
- `sock_fd`：Socket 文件描述符

**返回值**：
- 成功：返回读取的字节数
- 缓冲区满：返回 -2
- 连接关闭：返回 0
- 错误：返回 -1

**示例**：
```c
int n = buffer_read_from_socket(buf, sockfd);
if (n > 0) {
    printf("Received %d bytes\n", n);
} else if (n == 0) {
    printf("Connection closed\n");
}
```

### 5. 向 Socket 写入数据

```c
int buffer_write_to_socket(struct buffer *buf, int sock_fd);
```

**功能**：将缓冲区数据写入 Socket

**参数**：
- `buf`：缓冲区指针
- `sock_fd`：Socket 文件描述符

**返回值**：
- 成功：返回写入的字节数
- 缓冲区空：返回 -2
- 错误：返回 -1

**特点**：
- 循环写入直到缓冲区为空或发生错误
- 自动更新读指针

## 状态判断

### 缓冲区为空

```c
if (buf->size - buf->r_idx == buf->w_idx) {
    // 缓冲区为空
}
```

### 缓冲区已满

```c
if (buf->size - buf->r_idx == buf->w_idx) {
    // 缓冲区已满（与空判断相同，需要根据上下文区分）
}
```

### 可用读空间

```c
int readable_size = 0;
if (buf->w_idx >= buf->r_idx) {
    readable_size = buf->w_idx - buf->r_idx;
} else {
    readable_size = buf->size - buf->r_idx + buf->w_idx;
}
```

## 使用示例

### HTTP 服务器

```c
// server.c
void request_with_buffer(int sock_fd) {
    struct buffer *buf = new_buffer();

    // 从 socket 读取数据到缓冲区
    buffer_read_from_socket(buf, sock_fd);

    printf("Request:\n%s\n", buf->data);

    buffer_free(buf);
}

void response_with_buffer(int sock_fd) {
    struct buffer *buf = new_buffer();

    // 构建响应数据
    buffer_append(buf, "HTTP/1.1 200 OK\r\n", 17);
    buffer_append(buf, "Content-Type: text/html\r\n", 25);
    buffer_append(buf, "\r\n", 2);

    char *body = "<html><body>Hello</body></html>";
    buffer_append(buf, body, strlen(body));

    // 发送数据
    buffer_write_to_socket(buf, sock_fd);

    buffer_free(buf);
}
```

## 编译和运行

```bash
# 编译服务器
gcc -o server server.c buffer.c

# 编译客户端
gcc -o client client.c

# 运行服务器
./server
# Server is running at: 3000

# 运行客户端（另一个终端）
./client
```

## 使用 curl 测试

```bash
curl http://localhost:3000
```

## 注意事项

1. **内存管理**：使用完缓冲区后必须调用 `buffer_free()` 释放内存
2. **边界检查**：追加数据前应检查缓冲区是否有足够空间
3. **线程安全**：当前实现不是线程安全的，多线程环境需要加锁
4. **大小限制**：单个数据包不能超过 BUFFER_SIZE

## 扩展建议

1. **动态扩容**：当缓冲区满时自动扩容
2. **线程安全**：添加互斥锁保护
3. **数据统计**：添加字节计数器，统计收发流量
4. **引用计数**：支持多对象共享缓冲区

## 相关文件

- `buffer.h`：缓冲区接口定义
- `buffer.c`：缓冲区实现
- `server.c`：HTTP 服务器示例
- `client.c`：测试客户端

## 下一步学习

- **select 模块**：学习如何同时监控多个连接
- **poll 模块**：了解 select 的改进版本
- **epoll 模块**：掌握高性能 I/O 多路复用
