#include <iostream>
#include <chrono>
#include <atomic>
#include <string>
#include <vector>
#include <mutex>
#include <condition_variable>
#include <deque>
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
        //initialize
        GCPtr<Foo> foo1 = gcnew<Foo>();

        //collect
        int prevCount = count;
        std::size_t prevAllocSize = GC::getAllocSize();
        const std::size_t allocSize = GC::collect();

        //check
        check(allocSize == prevAllocSize, "No data should have been collected");
        check(count == prevCount, "No object should have been collected");
    });
}


void test2() {
    doTest("1 object, 1 collectable", []() {
        //initialize
        GCPtr<Foo> foo1 = gcnew<Foo>();
        foo1 = nullptr;

        //collect
        int prevCount = count;
        std::size_t prevAllocSize = GC::getAllocSize();
        const std::size_t allocSize = GC::collect();

        //check
        check(allocSize <= prevAllocSize, "Data should have been collected");
        check(prevCount - count == 1, "1 object should have been collected");
    });
}


void test3() {
    doTest("2 objects, 0 collectables, 0 cycles", []() {
        //initialize
        GCPtr<Foo> foo1 = gcnew<Foo>();
        foo1->other = gcnew<Foo>();

        //collect
        int prevCount = count;
        std::size_t prevAllocSize = GC::getAllocSize();
        const std::size_t allocSize = GC::collect();

        //check
        check(allocSize == prevAllocSize, "No data should have been collected");
        check(count == prevCount, "No object should have been collected");
    });
}


void test4() {
    doTest("2 objects, 0 collectables, 1 cycle", []() {
        //initialize
        GCPtr<Foo> foo1 = gcnew<Foo>();
        foo1->other = gcnew<Foo>();
        foo1->other->other = foo1;

        //collect
        int prevCount = count;
        std::size_t prevAllocSize = GC::getAllocSize();
        const std::size_t allocSize = GC::collect();

        //check
        check(allocSize == prevAllocSize, "No data should have been collected");
        check(count == prevCount, "No object should have been collected");
    });
}


void test5() {
    doTest("2 objects, 2 collectables, 0 cycles", []() {
        //initialize
        GCPtr<Foo> foo1 = gcnew<Foo>();
        foo1->other = gcnew<Foo>();
        foo1 = nullptr;

        //collect
        int prevCount = count;
        std::size_t prevAllocSize = GC::getAllocSize();
        const std::size_t allocSize = GC::collect();

        //check
        check(allocSize < prevAllocSize, "Data should have been collected");
        check(prevCount - count == 2, "2 objects should have been collected");
    });
}


void test6() {
    doTest("2 objects, 2 collectables, 1 cycle", []() {
        //initialize
        GCPtr<Foo> foo1 = gcnew<Foo>();
        foo1->other = gcnew<Foo>();
        foo1->other->other = foo1;
        foo1 = nullptr;

        //collect
        int prevCount = count;
        std::size_t prevAllocSize = GC::getAllocSize();
        const std::size_t allocSize = GC::collect();

        //check
        check(allocSize < prevAllocSize, "Data should have been collected");
        check(prevCount - count == 2, "2 objects should have been collected");
    });
}


struct Node {
    GCPtr<Node> left;
    GCPtr<Node> right;

    Node(std::size_t depth = 1) 
        : left(depth > 1 ? gcnew<Node>(depth - 1) : nullptr)
        , right(depth > 1 ? gcnew<Node>(depth - 1) : nullptr)
    {
    }
};


void test7() {
    doTest("1 thread, 2^20 objects per thread", []() {
        //initialize
        int prevCount = count;
        std::thread thread([]() {
            GCPtr<Node> root = gcnew<Node>(20);
        });
        thread.join();

        //collect
        const std::size_t allocSize = GC::collect();

        //wait for collections to finish
        waitCollection(prevCount);
    });
}


void test8() {
    doTest("4 threads, 2^20 objects per thread", []() {
        int prevCount = count;

        //initialize
        const std::size_t ThreadCount = 4;
        const std::size_t ObjectCountPerThread = 1 << 20;
        std::vector<std::thread> threads;

        for (std::size_t i = 0; i < ThreadCount; ++i) {
            threads.push_back(std::thread([]() {
                GCPtr<Node> root = gcnew<Node>(16);
            }));
        }

        for (std::thread& thread : threads) {
            thread.join();
        }

        //collect
        const std::size_t allocSize = GC::collect();

        //wait for collections to finish
        waitCollection(prevCount);
    });
}


void test9() {
    doTest("Producer-consumer, 2^20 objects", []() {
        int prevCount = count;

        //initialize
        const std::size_t ObjectCount = 1 << 20;
        std::mutex mutex;
        std::condition_variable cond;
        std::deque<GCPtr<Node>> objects;

        std::thread consumerThread([&]() {
            std::size_t receivedObjectCount = 0;
            while (receivedObjectCount < ObjectCount) {
                std::unique_lock lock(mutex);
                while (objects.empty()) {
                    cond.wait(lock);
                }
                objects.pop_front();
                ++receivedObjectCount;
            }
        });

        std::thread producerThread([&]() {
            for (std::size_t i = 0; i < ObjectCount; ++i) {
                {
                    std::lock_guard lock(mutex);
                    objects.push_back(gcnew<Node>());
                }
                cond.notify_one();
            }
        });

        producerThread.join();
        consumerThread.join();

        //collect
        const std::size_t allocSize = GC::collect();

        //wait for collections to finish
        waitCollection(prevCount);
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
    test7();
    test8();
    test9();
    
    if (errorCount > 0) {
        std::cout << "Errors: " << errorCount << std::endl;
    }
    else {
        std::cout << "All tests passed.\n";
    }
    
    system("pause");
    return errorCount;
}
