#include <iostream>
#include <atomic>
#include "gclib.hpp"


using namespace gclib;


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


static void test1() {
    Ptr<Foo> foo = make<Foo>();
    GC::collectGarbage();
}


int main() {
    GC::initialize();
    test1();
    system("pause");
    return 0;
}
