#include <iostream>
#include <chrono>
#include <atomic>
#include "gclib/gcnew.hpp"


template <class F> double timeFunction(F&& func) {
    auto start = std::chrono::high_resolution_clock::now();
    func();
    auto end = std::chrono::high_resolution_clock::now();
    return std::chrono::duration_cast<std::chrono::duration<double>>(end - start).count();
}


std::atomic<int> count{ 0 };


class Foo {
public:
    Foo() {
        count.fetch_add(1, std::memory_order_relaxed);
    }

    ~Foo() {
        count.fetch_sub(1, std::memory_order_relaxed);
    }
};


void test1() {
    GCPtr<Foo> foo1 = gcnew<Foo>();
}


int main() {
    test1();
    system("pause");
    return 0;
}
