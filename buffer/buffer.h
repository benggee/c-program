/**
 * @file buffer.h
 * @brief 循环缓冲区 (Circular Buffer) 接口定义
 *
 * 循环缓冲区是一种先进先出 (FIFO) 的数据结构，特别适合网络数据传输场景。
 * 它通过固定大小的数组和读写指针来实现数据的存储和读取，避免了频繁的内存分配和数据拷贝。
 *
 * 缓冲区状态：
 * - 空：r_idx == w_idx
 * - 满：size - r_idx == w_idx
 * - 可读空间：[r_idx, w_idx)
 */

#ifndef BUFFER_H
#define BUFFER_H

#define BUFFER_SIZE 65535  // 缓冲区大小 64KB

/**
 * @brief 循环缓冲区结构体
 */
struct buffer {
    char *data;    // 缓冲区数据指针
    int r_idx;     // 读指针 (read index)：指向下一个要读取的位置
    int w_idx;     // 写指针 (write index)：指向下一个要写入的位置
    int size;      // 缓冲区总大小
};

/**
 * @brief 创建一个新的循环缓冲区
 * @return 成功返回缓冲区指针，失败返回 NULL
 */
struct buffer *new_buffer();

/**
 * @brief 释放缓冲区及其内部内存
 * @param buf 要释放的缓冲区指针
 */
void buffer_free(struct buffer *buf);

/**
 * @brief 将数据追加到缓冲区
 * @param buf 缓冲区指针
 * @param data 要追加的数据
 * @param len 数据长度
 * @return 成功返回实际写入的字节数，缓冲区满返回 0
 */
int buffer_append(struct buffer *buf, char *data, size_t len);

/**
 * @brief 从 Socket 读取数据并存入缓冲区
 * @param buf 缓冲区指针
 * @param sock_fd Socket 文件描述符
 * @return 成功返回读取的字节数，缓冲区满返回 -2，连接关闭返回 0，错误返回 -1
 */
int buffer_read_from_socket(struct buffer *buf, int sock_fd);

/**
 * @brief 将缓冲区数据写入 Socket
 * @param buf 缓冲区指针
 * @param sock_fd Socket 文件描述符
 * @return 成功返回写入的字节数，缓冲区空返回 -2，错误返回 -1
 */
int buffer_write_to_socket(struct buffer *buf, int sock_fd);

#endif // BUFFER_H