#include <iostream>
#include <chrono>
#include <memory>
#include "gclib/make.hpp"
#include "gclib/MemoryResource.hpp"


using namespace gclib;


template <class F> double timeFunction(F&& func) {
    auto start = std::chrono::high_resolution_clock::now();
    func();
    auto end = std::chrono::high_resolution_clock::now();
    return std::chrono::duration_cast<std::chrono::duration<double>>(end - start).count();
}


class Foo {
public:
};


template <class T> class MemoryResourceAllocator {
public:
    MemoryResource mr;

    template <class U> struct rebind {
        typedef MemoryResourceAllocator<U> other;
    };

    void* allocate(size_t n) {

    }
};


void test0() {
    MemoryResourceAllocator<Foo> alloc;
    double dur = timeFunction([&]() {
        for (std::size_t i = 0; i < 5'000'000; ++i) {
            std::shared_ptr<Foo> foo1{ std::allocate_shared<Foo>(alloc) };
        }
    });
    std::cout << dur << " seconds\n";
}


void test1() {
    double dur = timeFunction([]() {
        for (std::size_t i = 0; i < 5'000'000; ++i) {
            VoidPtr foo1 = make<Foo>();
            dispose(foo1);
        }
    });
    std::cout << dur << " seconds\n";
}


int main() {
    test0();
    system("pause");
    return 0;
}
