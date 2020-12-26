#include <iostream>
#include <chrono>
#include <atomic>
#include "gclib/gcnew.hpp"
#include "gclib/GC.hpp"


template <class F> double timeFunction(F&& func) {
    auto start = std::chrono::high_resolution_clock::now();
    func();
    auto end = std::chrono::high_resolution_clock::now();
    return std::chrono::duration_cast<std::chrono::duration<double>>(end - start).count();
}


std::atomic<int> count{ 0 };


class Foo {
public:
    //GCPtr<Foo> other;

    Foo() {
        //count.fetch_add(1, std::memory_order_relaxed);
    }

    ~Foo() {
        //count.fetch_sub(1, std::memory_order_relaxed);
    }
};


void test1() {
    //{
        GCPtr<Foo> foo1 = gcnew<Foo>();
    //}
    GC::collect();
    std::cout << count << std::endl;
}


void test2() {
    double dur = timeFunction([]() {
        for (std::size_t i = 0; i < 4'000'000; ++i) {
            GCPtr<Foo> foo1 = gcnew<Foo>();
        }
    });
    std::cout << dur << " seconds\n";
}


int main() {
    test2();
    system("pause");
    return 0;
}
