#include <iostream>
#include <chrono>
#include <atomic>
#include <string>
#include "gclib/gcnew.hpp"
#include "gclib/GC.hpp"


template <class F> double timeFunction(F&& func) {
    auto start = std::chrono::high_resolution_clock::now();
    func();
    auto end = std::chrono::high_resolution_clock::now();
    return std::chrono::duration_cast<std::chrono::duration<double>>(end - start).count();
}


bool firstError = true;
std::size_t errorCount{ 0 };


template <class F> void doTest(const std::string& name, F&& func) {
    std::cout << "Test: " << name;
    firstError = true;
    const std::size_t prevErrorCount = errorCount;
    double dur = timeFunction(std::forward<F>(func));
    if (errorCount == prevErrorCount) {
        std::cout << ": OK. Duration: " << dur << " seconds.\n";
    }
    else {
        std::cout << "Errors: " << (errorCount - prevErrorCount);
        std::cout << ". Duration: " << dur << " seconds.\n\n";
    }
    GC::collect();
}


std::atomic<int> count{ 0 };


void waitCollection(int targetCount) {
    while (count.load(std::memory_order_acquire) > targetCount) {
        std::this_thread::yield();
    }
}


void check(bool b, const std::string& msg) {
    if (b) return;
    if (firstError) {
        std::cout << std::endl;
        firstError = false;
    }
    std::cout << "ERROR: " << msg << std::endl;
    ++errorCount;
}


class Foo {
public:
    GCPtr<Foo> other;

    Foo() {
        count.fetch_add(1, std::memory_order_relaxed);
    }

    ~Foo() {
        count.fetch_sub(1, std::memory_order_relaxed);
    }
};


void test1() {
    doTest("1 object, 0 collectables", []() {
        GCPtr<Foo> foo1 = gcnew<Foo>();
        std::size_t prevAllocSize = GC::getAllocSize();
        int prevCount = count;
        const std::size_t allocSize = GC::collect();
        check(allocSize == prevAllocSize, "No data should have been collected");
        check(count == prevCount, "No object should have been collected");
    });
}


void test2() {
    doTest("1 object, 1 collectable", []() {
        GCPtr<Foo> foo1 = gcnew<Foo>();
        std::size_t prevAllocSize = GC::getAllocSize();
        foo1 = nullptr;
        int prevCount = count;
        const std::size_t allocSize = GC::collect();
        waitCollection(0);
        check(allocSize <= prevAllocSize, "Data should have been collected");
        check(prevCount - count == 1, "1 object should have been collected");
    });
}


void test3() {
    doTest("2 objects, 0 collectables, 0 cycles", []() {
        GCPtr<Foo> foo1 = gcnew<Foo>();
        foo1->other = gcnew<Foo>();
        std::size_t prevAllocSize = GC::getAllocSize();
        int prevCount = count;
        const std::size_t allocSize = GC::collect();
        check(allocSize == prevAllocSize, "No data should have been collected");
        check(count == prevCount, "No object should have been collected");
    });
}


void test4() {
    doTest("2 objects, 0 collectables, 1 cycle", []() {
        GCPtr<Foo> foo1 = gcnew<Foo>();
        foo1->other = gcnew<Foo>();
        foo1->other->other = foo1;
        std::size_t prevAllocSize = GC::getAllocSize();
        int prevCount = count;
        const std::size_t allocSize = GC::collect();
        check(allocSize == prevAllocSize, "No data should have been collected");
        check(count == prevCount, "No object should have been collected");
    });
}


void test5() {
    doTest("2 objects, 2 collectables, 0 cycles", []() {
        GCPtr<Foo> foo1 = gcnew<Foo>();
        foo1->other = gcnew<Foo>();
        std::size_t prevAllocSize = GC::getAllocSize();
        foo1 = nullptr;
        int prevCount = count;
        const std::size_t allocSize = GC::collect();
        check(allocSize < prevAllocSize, "Data should have been collected");
        check(prevCount - count == 2, "2 objects should have been collected");
    });
}


void test6() {
    doTest("2 objects, 2 collectables, 1 cycle", []() {
        GCPtr<Foo> foo1 = gcnew<Foo>();
        foo1->other = gcnew<Foo>();
        foo1->other->other = foo1;
        std::size_t prevAllocSize = GC::getAllocSize();
        foo1 = nullptr;
        int prevCount = count;
        const std::size_t allocSize = GC::collect();
        check(allocSize < prevAllocSize, "Data should have been collected");
        check(prevCount - count == 2, "2 objects should have been collected");
    });
}


int main() {
    std::cout << std::fixed;
    test1();
    test2();
    test3();
    test4();
    test5();
    test6();
    if (errorCount > 0) {
        std::cout << "Errors: " << errorCount << std::endl;
    }
    system("pause");
    return errorCount;
}
