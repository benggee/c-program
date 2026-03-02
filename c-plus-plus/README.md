# C++ 特性演示模块

## 概述

本模块展示了现代 C++ 的重要特性和编程技巧，包括移动语义、运算符重载、线程池等。这些都是 C++11/14/17 引入的核心特性，对于编写高效、现代化的 C++ 代码至关重要。

## 目录结构

```
c-plus-plus/
├── xstring/              # 自定义字符串类（移动语义）
│   ├── xstring.hpp       # 类定义
│   └── main.cpp          # 示例代码
├── operator-reload/      # 运算符重载示例
│   └── main.cpp          # 复数类实现
└── thread/               # C++ 线程池
    ├── thread-pool.hpp   # 线程池实现
    └── main.cpp          # 示例代码
```

---

## 1. xstring - 移动语义演示

### 什么是移动语义？

移动语义（Move Semantics）是 C++11 引入的重要特性，它允许"转移"资源而不是"拷贝"资源，从而大幅提升性能。

### 拷贝 vs 移动

```cpp
// 拷贝：分配新内存，复制数据
xstring s1 = "hello";
xstring s2 = s1;  // 拷贝构造：深拷贝，涉及内存分配和数据复制

// 移动：直接转移资源所有权
xstring s1 = "hello";
xstring s2 = std::move(s1);  // 移动构造：仅转移指针，s1 不再拥有资源
```

### 核心概念

| 特性 | 拷贝语义 | 移动语义 |
|------|----------|----------|
| **操作** | 复制资源 | 转移资源所有权 |
| **原对象** | 保持有效 | 可能变为空 |
| **性能** | 低（需内存分配+复制） | 高（仅指针操作） |
| **使用场景** | 需要保留原对象 | 原对象不再使用 |

### 五种特殊成员函数

```cpp
class xstring {
public:
    // 1. 默认构造函数
    xstring() = default;

    // 2. 拷贝构造函数
    // 用于：xstring s2(s1);
    xstring(const xstring &s) {
        // 深拷贝：分配新内存，复制数据
        int len = strlen(s.m_str);
        m_str = new char[len + 1];
        strcpy(m_str, s.m_str);
        m_len = len;
        m_cap = len;
    }

    // 3. 移动构造函数 (C++11)
    // 用于：xstring s2(std::move(s1));
    xstring(xstring&& s) {
        // 转移资源：直接偷取指针
        m_str = s.m_str;
        m_len = s.m_len;
        m_cap = s.m_cap;
        s.m_str = nullptr;  // 原对象不再拥有资源
    }

    // 4. 拷贝赋值运算符
    // 用于：s2 = s1;
    xstring& operator=(const xstring &s) {
        if (this != &s) {
            delete[] m_str;  // 释放旧资源

            // 深拷贝
            int len = strlen(s.m_str);
            m_str = new char[len + 1];
            strcpy(m_str, s.m_str);
            m_len = len;
            m_cap = len;
        }
        return *this;
    }

    // 5. 移动赋值运算符 (C++11)
    // 用于：s2 = std::move(s1);
    xstring& operator=(xstring&& s) {
        if (this != &s) {
            delete[] m_str;  // 释放旧资源

            // 转移资源
            m_str = s.m_str;
            m_len = s.m_len;
            m_cap = s.cap;
            s.m_str = nullptr;  // 原对象不再拥有资源
        }
        return *this;
    }

    // 6. 析构函数
    ~xstring() {
        delete[] m_str;  // 释放资源
        m_str = nullptr;
    }

private:
    char* m_str;
    size_t m_len;
    size_t m_cap;
};
```

### Rule of Five（五法则）

如果需要定义以下任何一个函数，应该定义全部五个：

1. 析构函数
2. 拷贝构造函数
3. 拷贝赋值运算符
4. 移动构造函数 (C++11)
5. 移动赋值运算符 (C++11)

### 编译和运行

```bash
g++ -std=c++11 -o test main.cpp
./test
```

---

## 2. operator-reload - 运算符重载

### 什么是运算符重载？

运算符重载允许为自定义类型定义运算符的行为，使代码更直观、更易读。

### 复数类实现

完整的复数类，支持各种运算符重载：

```cpp
class complex {
private:
    float m_c;   // 实部
    float m_ci;  // 虚部

public:
    // 算术运算符
    complex operator+(const complex& c);
    complex operator-(const complex &c) const;
    complex operator*(const complex &c) const;
    complex operator/(const complex &c);

    // 复合赋值运算符
    complex& operator+=(const complex &c);
    complex& operator-=(const complex &c);
    complex& operator*=(const complex &c);
    complex& operator/=(const complex &c);

    // 关系运算符
    bool operator==(const complex &c);
    bool operator!=(const complex &c);
    bool operator>(const complex &c);
    bool operator>=(const complex &c);
    bool operator<(const complex &c);
    bool operator<=(const complex &c);

    // 自增/自减运算符
    complex& operator++();        // 前置 ++
    complex operator++(int);      // 后置 ++
    complex& operator--();        // 前置 --
    complex operator--(int);      // 后置 --
};
```

### 可重载的运算符

| 类型 | 运算符 |
|------|--------|
| **算术** | `+ - * / %` |
| **关系** | `== != < > <= >=` |
| **逻辑** | `&& || !` |
| **位运算** | `& | ^ ~ << >>` |
| **赋值** | `= += -= *= /= %= &= \|= ^= <<= >>=` |
| **自增/自减** | `++ --` |
| **下标** | `[]` |
| **函数调用** | `()` |
| **成员访问** | `-> ->*` |
| **内存管理** | `new delete` |

### 不可重载的运算符

| 运算符 | 说明 |
|--------|------|
| `::` | 作用域解析 |
| `.` | 成员访问 |
| `.*` | 成员指针访问 |
| `?:` | 三元条件 |
| `sizeof` | 大小 |
| `typeid` | 类型信息 |

### 返回值类型选择

```cpp
// 返回新对象：按值返回
complex operator+(const complex& c) {
    return complex(m_c + c.m_c, m_ci + c.m_ci);
}

// 返回引用：支持链式调用
complex& operator+=(const complex &c) {
    m_c += c.m_c;
    m_ci += c.m_ci;
    return *this;
}

// const 成员函数：不修改对象
complex operator-(const complex &c) const {
    return complex(m_c - c.m_c, m_ci - c.m_ci);
}
```

### 前置 vs 后置自增

```cpp
// 前置：返回引用，效率更高
complex& operator++() {
    m_c++;
    m_ci++;
    return *this;  // 返回修改后的对象
}

// 后置：返回旧值，参数 int 是标记
complex operator++(int) {
    complex old = *this;  // 保存旧值
    m_c++;
    m_ci++;
    return old;  // 返回旧值
}
```

### 编译和运行

```bash
g++ -std=c++11 -o test main.cpp
./test
```

---

## 3. thread - C++ 线程池

### 什么是线程池？

线程池是一种并发设计模式，它维护多个工作线程，可以重复使用这些线程来执行任务，避免频繁创建和销毁线程的开销。

### 线程池架构

```
┌─────────────────────────────────────────────┐
│              主线程 (Main Thread)            │
│                                               │
│  ┌─────────┐         ┌─────────┐           │
│  │ 提交任务 │ ──────> │ 任务队列 │           │
│  └─────────┘         └─────────┘           │
│      │                    │                │
│      │               条件变量                │
│      │                    │                │
│      └────────────────────┼────────────────┘
│                           ▼
│  ┌─────────────────────────────────────┐  │
│  │         Worker 线程池                │  │
│  │  ┌─────┐  ┌─────┐  ┌─────┐         │  │
│  │  │ W1  │  │ W2  │  │ W3  │  ...    │  │
│  │  └─────┘  └─────┘  └─────┘         │  │
│  │     等待任务 -> 执行任务             │  │
│  └─────────────────────────────────────┘  │
└─────────────────────────────────────────────┘
```

### 核心实现

```cpp
class TPool {
private:
    std::queue<TaskFunc> m_tasks;          // 任务队列
    std::vector<std::thread*> m_threads;   // 线程数组
    std::mutex m_mutex;                    // 互斥锁
    std::condition_variable m_cond;        // 条件变量
    size_t m_thread_size;                  // 线程数
    bool m_terminate;                      // 终止标志
    std::atomic<int> m_atomic{0};          // 原子计数器

public:
    // 初始化线程池
    bool init(size_t size) {
        m_thread_size = size;
        return true;
    }

    // 启动线程池
    bool start() {
        for (size_t i = 0; i < m_thread_size; i++) {
            m_threads.push_back(new std::thread(&TPool::run, this));
        }
        return true;
    }

    // 提交任务
    template <class F, class... A>
    auto exec(F&& f, A&&... args)
        -> std::future<decltype(f(args...))>
    {
        using retType = decltype(f(args...));

        // 创建 packaged_task
        auto task = std::make_shared<std::packaged_task<retType()>>(
            std::bind(std::forward<F>(f), std::forward<A>(args)...)
        );

        // 包装任务
        TaskFunc fPtr = std::make_shared<Task>();
        fPtr->m_func = [task]() { (*task)(); };

        // 加入队列
        std::unique_lock<std::mutex> lock(m_mutex);
        m_tasks.push(fPtr);
        m_cond.notify_one();  // 唤醒一个工作线程

        return task->get_future();  // 返回 future
    }

    // 等待所有任务完成
    bool waitDone() {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_cond.wait(lock, [this] { return m_tasks.empty(); });
        return true;
    }

private:
    // 工作线程函数
    bool run() {
        while (!m_terminate) {
            TaskFunc task;
            bool ok = get(task);  // 获取任务
            if (ok) {
                ++m_atomic;       // 增加活跃线程计数
                task->m_func();   // 执行任务
                --m_atomic;       // 减少活跃线程计数

                // 如果所有任务完成，通知等待线程
                std::unique_lock<std::mutex> lock(m_mutex);
                if (m_atomic == 0 && m_tasks.empty()) {
                    m_cond.notify_all();
                }
            }
        }
    }

    // 从队列获取任务
    bool get(TaskFunc &t) {
        std::unique_lock<std::mutex> lock(m_mutex);
        if (m_tasks.empty()) {
            m_cond.wait(lock, [this] {
                return m_terminate || !m_tasks.empty();
            });
        }

        if (m_terminate) return false;

        if (!m_tasks.empty()) {
            t = std::move(m_tasks.front());
            m_tasks.pop();
            return true;
        }
        return false;
    }
};
```

### 使用示例

```cpp
// 普通函数
void threadFunc(int a) {
    cout << "a=" << a << endl;
}

// 类成员函数
class A {
public:
    int run(int a, int b) {
        return a + b;
    }
};

int main() {
    TPool pool;
    pool.init(10);   // 10 个工作线程
    pool.start();    // 启动线程池

    // 提交普通函数
    pool.exec(threadFunc, 100);
    pool.exec(threadFunc, 200);

    // 提交成员函数
    A a1;
    auto future = pool.exec(
        std::bind(&A::run, &a1,
                  std::placeholders::_1,
                  std::placeholders::_2),
        10, 20
    );
    int result = future.get();  // 获取结果
    cout << "result: " << result << endl;

    pool.waitDone();  // 等待所有任务完成
    return 0;
}
```

### 关键技术点

#### 1. std::packaged_task

包装任务，使其可以异步执行并返回结果：

```cpp
auto task = std::make_shared<std::packaged_task<retType()>>(
    std::bind(std::forward<F>(f), std::forward<A>(args)...)
);
```

#### 2. std::future

获取异步任务的结果：

```cpp
auto future = pool.exec(...);
int result = future.get();  // 阻塞等待结果
```

#### 3. 条件变量

实现生产者-消费者模式：

```cpp
// 生产者（主线程）
m_tasks.push(task);
m_cond.notify_one();  // 通知有新任务

// 消费者（工作线程）
m_cond.wait(lock, [this] {
    return m_terminate || !m_tasks.empty();
});
```

#### 4. 原子操作

线程安全的计数器：

```cpp
std::atomic<int> m_atomic{0};

++m_atomic;  // 原子自增
--m_atomic;  // 原子自减
```

### 编译和运行

```bash
g++ -std=c++11 -pthread -o test main.cpp
./test
```

---

## 学习路径

建议按以下顺序学习：

1. **xstring** → 理解移动语义的基本概念
2. **operator-reload** → 学习运算符重载
3. **thread** → 掌握现代 C++ 并发编程

## 参考资料

- [C++ Reference - std::move](https://en.cppreference.com/w/cpp/utility/move)
- [C++ Reference - std::future](https://en.cppreference.com/w/cpp/thread/future)
- [C++ Reference - std::packaged_task](https://en.cppreference.com/w/cpp/thread/packaged_task)
- [C++ Concurrency in Action](https://www.manning.com/books/c-plus-plus-concurrency-in-action-second-edition)
