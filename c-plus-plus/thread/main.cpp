#include "thread-pool.hpp"
#include <iostream>

using namespace std;

void threadFunc(int a) {
   cout << "a=" << a << endl;
}

class A {
public:
    A() = default;
    int run(int a, int b) {
        return a + b;
    }
};

int main() {
    TPool p1;
    p1.init(10);
    p1.start();
    p1.exec(threadFunc, 100);
    p1.exec(threadFunc, 200);

    A a1;
    auto fu1 = p1.exec(std::bind(&A::run, &a1, std::placeholders::_1, std::placeholders::_2), 10, 20);
    int ret = fu1.get();
    std::cout << "res:" << ret << std::endl;

    p1.waitDone();
    return 0;
}