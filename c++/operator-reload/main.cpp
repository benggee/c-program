#include <iostream>

using namespace std;

class complex
{
public:
    complex() = default;
    complex(float c, float ci)
    {
        m_c = c;
        m_ci = ci;

        cout << "this is constructor" << endl;
    }
    // 拷贝构造
    complex(const complex &c)
    {
        m_c = c.m_c;
        m_ci = c.m_ci;

        cout << "this is copy constructor" << endl;
    }

    ~complex() = default;

public:
    complex& operator= (const complex& c)
    {
        if (this != &c)
        {
            m_c = c.m_c;
            m_ci = c.m_ci;
        }
        return *this;
    }

    complex operator+(const complex& c)
    {
        complex tmp;
        tmp.m_c = m_c + c.m_c;
        tmp.m_ci = m_ci + c.m_ci;
        return tmp;
    }

    complex& operator+=(const complex &c) {
        m_c += c.m_c;
        m_ci += c.m_ci;

        return *this;
    }

    complex operator-(const complex &c) const {
        return complex(m_c - c.m_c, m_ci - c.m_ci);
    }

    complex& operator-=(const complex &c) {
        m_c -= c.m_c;
        m_ci -= c.m_ci;

        return *this;
    }

    complex operator*(const complex &c) const {
        return complex(m_c * c.m_c - m_ci*c.m_ci, m_c*c.m_ci + m_ci*c.m_c);
    }

    complex& operator*=(const complex &c) {
        complex tmp(*this);  // 拷贝构造
        m_c = tmp.m_c*c.m_c - m_ci*c.m_ci;
        m_ci = tmp.m_c*c.m_ci + tmp.m_ci*c.m_c;
        return *this;
    }

    complex operator/(const complex &c) {
        double t = c.m_c*c.m_c + c.m_ci*c.m_ci;
        return complex((m_c*c.m_c - m_ci*(-c.m_ci)) / t, (m_c*(-c.m_ci) + m_ci*c.m_c) / t);
    }

    complex& operator/=(const complex &c) {
        complex tmp(*this);  //拷贝构造函数
        double t = c.m_c*c.m_c + c.m_ci*c.m_ci;
        m_c = (tmp.m_c*c.m_c - tmp.m_ci*(-c.m_ci)) / t;
        m_ci = (tmp.m_c*(-c.m_c) + tmp.m_ci*c.m_c) / t;
        return *this;
    }

    bool operator==(const complex &c) {
        return (m_c == c.m_c) && (m_ci == c.m_ci);
    }

    bool operator!=(const complex &c) {
        return !((m_c == c.m_c) && (m_ci == c.m_ci));
    }

    bool operator>(const complex &c) {
        return (m_c > c.m_c) && (m_ci > c.m_ci);
    }

    bool operator>=(const complex &c) {
        return (m_c >= c.m_c) && (m_ci >= c.m_ci);
    }

    bool operator<(const complex &c) {
        return (m_c < c.m_c) && (m_ci < c.m_ci);
    }

    bool operator<=(const complex &c) {
        return (m_c <= c.m_c) && (m_ci <= c.m_ci);
    }

    // 前置++
    complex& operator++() {
        m_c++;
        m_ci++;
        return *this;
    }

    // 后置++
    complex operator++(int) {
        return complex(m_c++, m_ci++);
    }

    // 前置--
    complex& operator--() {
        m_c--;
        m_ci--;
        return *this;
    }

    // 后置--
    complex operator--(int) {
        return complex(m_c--, m_ci--);
    }

private:
    float m_c = 0;
    float m_ci = 0;
};

int main() {
    complex c1(1, 2);
    complex c2(3, 4);

    complex c3;
    c3 = c1 + c2;

    return 0;
}