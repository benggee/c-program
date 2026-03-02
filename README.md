# C Program - 网络编程系列教程示例代码

这是一个关于 C 语言网络编程的教学项目，包含了从基础的 TCP 通信到高级 I/O 多路复用技术的完整实现。

## 项目概述

本项目展示了网络编程的核心概念和技术，包括：

- **基础 TCP 服务器**：使用阻塞式 I/O 的简单服务器
- **缓冲区管理**：循环缓冲区实现，用于高效数据传输
- **I/O 多路复用**：select、poll、epoll 三种技术的实现对比
- **非阻塞 Socket**：配合线程池的高性能服务器架构
- **HTTP 服务器**：基于 epoll 的 HTTP 服务器实现
- **C++ 特性**：现代 C++ 特性演示（移动语义、运算符重载等）

## 项目结构

```
c-program/
├── buffer/                 # 循环缓冲区实现
│   ├── buffer.h            # 缓冲区接口定义
│   ├── buffer.c            # 缓冲区实现
│   ├── server.c            # 使用缓冲区的服务器
│   └── client.c            # 测试客户端
├── select/                 # select I/O 多路复用
│   └── select.c            # select 实现
├── poll/                   # poll I/O 多路复用
│   ├── poll.c              # poll 实现
│   └── http-server/        # 基于 poll 的 HTTP 服务器
├── epoll/                  # epoll I/O 多路复用
│   ├── epoll.c             # epoll 实现
│   └── http-server/        # 基于 epoll 的 HTTP 服务器
├── non-blocking-socket/    # 非阻塞 Socket + 线程池
│   ├── buffer.h/c          # 缓冲区实现
│   ├── buffer_map.h/c      # Socket 缓冲区映射
│   ├── thread_pool.h/c     # 线程池实现
│   ├── tcp_server.h/c      # TCP 服务器
│   └── server.c            # 服务器入口
├── cmake/                  # 使用 CMake 构建的项目
│   └── lib/                # 可复用库代码
└── c-plus-plus/            # C++ 特性演示
    ├── xstring/            # 自定义字符串类
    ├── operator-reload/    # 运算符重载
    └── thread/             # C++ 线程
```

## 核心概念

### 1. 循环缓冲区 (Circular Buffer)

循环缓冲区是一种高效的数据结构，特别适合网络数据传输场景：

- **优势**：避免频繁的内存分配和数据拷贝
- **实现**：使用读指针和写指针管理数据
- **应用**：TCP 接收和发送缓冲

详见 `buffer/` 目录。

### 2. I/O 多路复用

I/O 多路复用允许单个线程同时监控多个文件描述符：

| 技术 | 特点 | 适用场景 |
|------|------|----------|
| **select** | 跨平台，有 FD 数量限制（通常 1024） | 移植性要求高的场景 |
| **poll** | 跨平台，无 FD 数量限制 | 需要监控大量 FD 的场景 |
| **epoll** | Linux 特有，性能最优，支持边缘触发 | 高性能 Linux 服务器 |

### 3. 非阻塞 Socket + 线程池

非阻塞 I/O 配合线程池可以构建高性能服务器：

- **Accepter 线程**：负责接受新连接
- **Worker 线程池**：处理具体业务逻辑
- **任务队列**：使用条件变量进行任务分发

### 4. Reactor 模式

epoll HTTP 服务器采用了 Reactor 模式：

- **事件注册**：关注可读/可写事件
- **事件分发**：epoll_wait 返回就绪的 FD
- **事件处理**：对应的回调函数处理业务

## 编译和运行

### 环境要求

- GCC 或 Clang 编译器
- Linux 或 macOS 系统
- CMake（用于 cmake/ 目录下的项目）
- Make（可选，用于其他项目）

### 编译示例

#### Buffer 模块

```bash
cd buffer
gcc -o server server.c buffer.c
gcc -o client client.c buffer.c
./server 8080
# 另一个终端
./client localhost 8080
```

#### Select 模块

```bash
cd select
gcc -o server select.c
./server 8080
```

#### Poll 模块

```bash
cd poll
gcc -o server poll.c
./server 8080
```

#### Epoll 模块

```bash
cd epoll
gcc -o server epoll.c
./server 8080
```

#### Epoll HTTP 服务器

```bash
cd epoll/http-server
mkdir -p build && cd build
cmake ..
make
./http_server 8080
# 测试
curl http://localhost:8080
```

#### 非阻塞 Socket + 线程池

```bash
cd non-blocking-socket
mkdir -p build && cd build
cmake ..
make
./server 8080
```

#### CMake 项目

```bash
cd cmake
mkdir -p build && cd build
cmake ..
make
./tcp_server 8080
# 测试
curl http://localhost:8080
```

#### C++ 示例

```bash
cd c-plus-plus/xstring
g++ -std=c++11 -o test xstring.cpp
./test
```

## 测试工具

使用以下工具测试服务器：

```bash
# 使用 netcat
echo "hello" | nc localhost 8080

# 使用 curl (HTTP 服务器)
curl http://localhost:8080

# 使用 ab (Apache Bench) 压力测试
ab -n 10000 -c 100 http://localhost:8080/
```

## 性能对比

三种 I/O 多路复用技术的性能对比（仅供参考）：

| 技术 | 连接数 | CPU 使用率 | 延迟 | 适合场景 |
|------|--------|------------|------|----------|
| select | < 1000 | 中 | 中 | 小规模，跨平台 |
| poll | < 10000 | 中高 | 中 | 中规模，跨平台 |
| epoll | > 100000 | 低 | 低 | 大规模，Linux |

## 学习路径

建议按以下顺序学习：

1. **基础 TCP** → 理解 socket 编程基础
2. **Buffer** → 学习数据缓冲技巧
3. **Select** → 理解 I/O 多路复用概念
4. **Poll** → 了解 select 的改进版本
5. **Epoll** → 掌握高性能网络编程
6. **非阻塞 Socket** → 综合运用多种技术
7. **HTTP 服务器** → 实际应用场景

## 常见问题

### Q: 为什么 epoll 性能更好？

A: epoll 使用了红黑树存储监控的 FD，返回时就绪的 FD 列表，时间复杂度为 O(1)。而 select/poll 每次都需要线性遍历所有 FD，复杂度为 O(n)。

### Q: 什么是边缘触发 (ET) 和水平触发 (LT)？

A:
- **LT (Level-Triggered)**：默认模式，只要条件满足就一直通知
- **ET (Edge-Triggered)**：只在状态变化时通知一次，效率更高但编程更复杂

### Q: 如何选择合适的 I/O 模型？

A:
- 少量连接：阻塞 I/O 最简单
- 中等规模连接：poll 足够
- 大规模连接：epoll 是最佳选择

## 贡献

欢迎提交 Issue 和 Pull Request！

## 许可证

本项目采用 MIT 许可证，详见 [LICENSE](LICENSE) 文件。

## 参考资料

- [Linux epoll 官方文档](https://man7.org/linux/man-pages/man7/epoll.7.html)
- [Beej's Guide to Network Programming](https://beej.us/guide/bgnet/)
- [Unix Network Programming (Volume 1)](https://en.wikipedia.org/wiki/Unix_Network_Programming)
