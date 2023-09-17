#include <stdlib.h>
#include <string.h>

class xstring
{
public:
    size_t len() const {
        return m_len;
    }

    size_t cap() const {
        return m_cap;
    }

public:
    friend std::ostream& operator<<(std::ostream& os, const xstring& s) {
        return std::cout << s.m_str;
    }

public:
    xstring() = default;

    xstring(const char *s) {
        if (s == nullptr) {
            m_str = new char[1];
            if (m_str == nullptr) {
                exit(-1);
            }
            *m_str = '\0';
            m_len = 0;
            m_cap = 0;
        } else {
            int len = strlen(s);
            m_str = new char[len + 1];
            if (m_str == nullptr) {
                exit(-1);
            }

            strcpy(m_str, s);
            m_len = len;
            m_cap = len;
        }
    }

    xstring(const size_t cap) {
        m_str = new char[cap + 1];
        if (m_str == nullptr) {
            exit(-1);
        }
        m_len = 0;
        m_cap = cap;
    }

    // 拷贝构造函数
    // 拷贝构造用于一个对象创建另一个对象
    xstring(const xstring &s) {
        std::cout << "this is copy construct" << std::endl;

        int len = strlen(s.m_str);
        m_str = new char[len + 1];
        if (m_str == nullptr) {
            exit(-1);
        }
        strcpy(m_str, s.m_str);
        m_len = len;
        m_cap = len;
    }

    // 转移构造函数
    // 一个临时对象的值移动到一个新的对象上
    // 一个对象创建另一个对象，但是创建后原对象不再使用
    // 使用转移构造函数可以避免内存拷贝的开销
    xstring(xstring&& s) {
        std::cout << "this is move construct" << std::endl;

        if (s.m_str != nullptr) {
            m_str = s.m_str;
            m_len = s.m_len;
            m_cap = s.m_cap;
            s.m_str = nullptr;
        }
    }

    // 拷贝赋值函数
    // 将一个对象赋值给另一个对象
    xstring& operator=(const xstring &s) {
        if (this != &s) {
            delete[] m_str;

            int len = strlen(s.m_str);
            if (m_cap < len) {
                m_str = new char[len + 1];
                if (m_str == nullptr) {
                    exit(-1);
                }
                m_len = len;
                m_cap = len;
            } else {
                memset(m_str, 0, len + 1);
                m_len = len;
                m_cap = s.m_cap;
            }
            strcpy(m_str, s.m_str);
        }

        return *this;
    }

    // 移动赋值函数
    // 将一个临时对象的值移动到当前对象中，可以避免内存拷贝的开销
    xstring& operator=(xstring&& s) {
       if (this != &s) {
           delete[] m_str;
           m_str = s.m_str;
           m_len = s.m_len;
           m_cap = s.m_cap;
           s.m_str = nullptr;
       }

       return *this;
    }

    // 析构函数
    ~xstring() {
        delete[] m_str;
        m_str = nullptr;
    }

private:
    char* m_str;
    size_t m_len;
    size_t m_cap;
};