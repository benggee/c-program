//
// Created by EDY on 2023/10/10.
//
#include <iostream>
#include <queue>
#include <vector>
#include <future>

class TPool {
public:
    TPool(): m_thread_size(1), m_terminate(false) {};
    ~TPool() { stop(); }
    bool init(size_t size) {
        std::unique_lock<std::mutex> lock(m_mutex);
        if (!m_threads.empty()) {
            return false;
        }
        m_thread_size = size;
        return true;
    }
    void stop() {
        {
            std::unique_lock<std::mutex> lock(m_mutex);
            m_terminate = true;
            m_cond.notify_all();
        }

        for (auto & m_thread : m_threads) {
            if (m_thread->joinable()) {
                m_thread->join();
            }
            delete m_thread;
            m_thread = nullptr;
        }

        std::unique_lock<std::mutex> lock(m_mutex);
        m_threads.clear();
    }
    bool start() {
        std::unique_lock<std::mutex> lock(m_mutex);
        if (!m_threads.empty()) {
            return false;
        }

        for (size_t i = 0; i < m_thread_size; i++) {
            m_threads.push_back(new std::thread(&TPool::run, this));
        }
        return true;
    }

    template <class F, class... A>
    auto exec(F&& f, A&&... args)->std::future<decltype(f(args...))> {
        using retType = decltype(f(args...));
        auto task = std::make_shared<std::packaged_task<retType()>>(std::bind(std::forward<F>(f), std::forward<A>(args)...));
        TaskFunc fPtr = std::make_shared<Task>();
        fPtr->m_func = [task](){
            (*task)();
        };

        std::unique_lock<std::mutex> lock(m_mutex);
        m_tasks.push(fPtr);
        m_cond.notify_one();

        return task->get_future();
    }
    bool waitDone() {
        std::unique_lock<std::mutex> lock(m_mutex);

        if (m_tasks.empty())
            return false;

        m_cond.wait(lock, [this]{return m_tasks.empty();});
        return true;
    }

private:
    struct Task {
        Task() {}
        std::function<void()> m_func;
    };

    typedef std::shared_ptr<Task> TaskFunc;

private:
    bool get(TaskFunc &t) {
        std::unique_lock<std::mutex> lock(m_mutex);
        if (m_tasks.empty()) {
            m_cond.wait(lock, [this]{return m_terminate || !m_tasks.empty();});
        }

        if (m_terminate)
            return false;

        if (!m_tasks.empty()) {
            t = std::move(m_tasks.front());
            m_tasks.pop();
            return true;
        }

        return false;
    }
    bool run() {
        while(!m_terminate) {
            TaskFunc task;
            bool ok = get(task);
            if (ok) {
                ++m_atomic;

                task->m_func();

                --m_atomic;

                std::unique_lock<std::mutex> lock(m_mutex);
                if (m_atomic == 0 && m_tasks.empty()) { // 是否所有任务都执行完成
                    m_cond.notify_all();
                }
            }
        }
    }

private:
    std::queue<TaskFunc> m_tasks;
    std::vector<std::thread*> m_threads;
    std::mutex m_mutex;
    std::condition_variable m_cond;
    size_t m_thread_size;
    bool m_terminate;
    std::atomic<int> m_atomic{0};
};
