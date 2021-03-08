#include <iostream>
#include <chrono>
#include <atomic>
#include <string>
#include <vector>
#include <mutex>
#include <condition_variable>
#include <deque>
#include "gclib.hpp"


template <class F> double timeFunction(F&& func) {
    auto start = std::chrono::high_resolution_clock::now();
    func();
    auto end = std::chrono::high_resolution_clock::now();
    return std::chrono::duration_cast<std::chrono::duration<double>>(end - start).count();
}


bool firstError = true;
size_t errorCount{ 0 };


template <class F> void doTest(const std::string& name, F&& func) {
    std::cout << "Test: " << name;
    firstError = true;
    const size_t prevErrorCount = errorCount;
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

    //void* operator new(size_t s) {
    //    return ::operator new(s);
    //}

    //void operator delete(void* mem) {
    //    ::operator delete(mem);
    //}
};


void test1() {
    doTest("1 object, 0 collectables", []() {
        //initialize
        GCPtr<Foo> foo1 = gcnew<Foo>();

        //collect
        int prevCount = count;
        size_t prevAllocSize = GC::getAllocSize();
        const size_t allocSize = GC::collect();

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
        size_t prevAllocSize = GC::getAllocSize();
        const size_t allocSize = GC::collect();

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
        size_t prevAllocSize = GC::getAllocSize();
        const size_t allocSize = GC::collect();

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
        size_t prevAllocSize = GC::getAllocSize();
        const size_t allocSize = GC::collect();

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
        size_t prevAllocSize = GC::getAllocSize();
        const size_t allocSize = GC::collect();

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
        size_t prevAllocSize = GC::getAllocSize();
        const size_t allocSize = GC::collect();

        //check
        check(allocSize < prevAllocSize, "Data should have been collected");
        check(prevCount - count == 2, "2 objects should have been collected");
    });
}


struct Node {
    GCPtr<Node> left;
    GCPtr<Node> right;
    int data;

    Node(size_t depth = 1, int data = 0) 
        : left(depth > 1 ? gcnew<Node>(depth - 1) : nullptr)
        , right(depth > 1 ? gcnew<Node>(depth - 1) : nullptr)
        , data(data)
    {
        if (depth == 0) {
            throw std::invalid_argument("depth");
        }
        count.fetch_add(1, std::memory_order_relaxed);
    }

    ~Node() {
        count.fetch_sub(1, std::memory_order_relaxed);
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
        const size_t allocSize = GC::collect();

        //wait for collections to finish
        waitCollection(prevCount);
    });
}


void test8() {
    doTest("4 threads, 2^20 objects per thread", []() {
        int prevCount = count;

        //initialize
        const size_t ThreadCount = 4;
        const size_t ObjectCountPerThread = 1 << 20;
        std::vector<std::thread> threads;

        for (size_t i = 0; i < ThreadCount; ++i) {
            threads.push_back(std::thread([]() {
                GCPtr<Node> root = gcnew<Node>(16);
            }));
        }

        for (std::thread& thread : threads) {
            thread.join();
        }

        //collect
        const size_t allocSize = GC::collect();

        //wait for collections to finish
        waitCollection(prevCount);
    });
}


void test9() {
    doTest("Producer-consumer, 2^20 objects", []() {
        int prevCount = count;

        //initialize
        const size_t ObjectCount = 1 << 20;
        std::mutex mutex;
        std::condition_variable cond;
        std::deque<GCPtr<Node>> objects;

        std::thread consumerThread([&]() {
            size_t receivedObjectCount = 0;
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
            for (size_t i = 0; i < ObjectCount; ++i) {
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
        const size_t allocSize = GC::collect();

        //wait for collections to finish
        waitCollection(prevCount);
    });
}


void test10() {
    doTest("pointer to middle of object, 0 collectables", []() {
        //initialize
        GCPtr<Node> node1 = gcnew<Node>();
        GCPtr<int> data1 = &node1->data;
        node1 = nullptr;

        //collect
        int prevCount = count;
        size_t prevAllocSize = GC::getAllocSize();
        const size_t allocSize = GC::collect();

        //check
        check(allocSize == prevAllocSize, "No data should have been collected");
        check(prevCount == count, "Notobjects should have been collected");
    });
}


void test11() {
    doTest("pointer to middle of object, 1 collectable", []() {
        //initialize
        GCPtr<Node> node1 = gcnew<Node>();
        GCPtr<int> data1 = &node1->data;
        node1 = nullptr;
        data1 = nullptr;

        //collect
        int prevCount = count;
        size_t prevAllocSize = GC::getAllocSize();
        const size_t allocSize = GC::collect();

        //check
        check(allocSize < prevAllocSize, "Data should have been collected");
        check(prevCount - count == 1, "1 object should have been collected");
    });
}


void test12() {
    doTest("exception during construction", []() {
        size_t prevAllocSize = GC::getAllocSize();

        //initialize
        try {
            GCPtr<Node> node1 = gcnew<Node>(0);
        }
        catch (const std::invalid_argument&) {
        }

        //collect
        const size_t allocSize = GC::collect();

        //check
        check(allocSize == prevAllocSize, "Data should not have been collected");
    });
}


void test13() {
    doTest("garbage-collected array", []() {
        size_t prevAllocSize = GC::getAllocSize();
        int prevCount = count;

        //initialize
        GCPtr<Node> nodeArray = gcnewArray<Node>(10, 1);

        //check
        check(count == prevCount + 10, "Array not initialized correctly");

        //try to collect
        size_t allocSize = GC::collect();

        //check
        check(allocSize > prevAllocSize, "Data should not have been collected");
        check(count == prevCount + 10, "Array objects should not have been destroyed");

        //try to collect from pointer to middle of array
        nodeArray += 5;
        allocSize = GC::collect();

        //check
        check(allocSize > prevAllocSize, "Data should not have been collected (2)");
        check(count == prevCount + 10, "Array objects should not have been destroyed (2)");

        //collect
        nodeArray = nullptr;
        allocSize = GC::collect();

        //check
        check(allocSize == prevAllocSize, "Data not collected correctly");
        check(count == prevCount, "Array objects not destroyed correctly");
    });
}


void test14() {
    doTest("gcdelete", []() {
        size_t prevAllocSize = GC::getAllocSize();
        int prevCount = count;

        //initialize
        GCPtr<Node> node = gcnew<Node>(1);
        GCPtr<Node> nodeArray = gcnewArray<Node>(10, 1);

        //check
        check(count == prevCount + 11, "Objects not initialized correctly");

        //try to collect
        size_t allocSize = GC::collect();

        //check
        check(allocSize > prevAllocSize, "Data should not have been collected");
        check(count == prevCount + 11, "Objects should not have been destroyed");

        //delete objects
        gcdelete(std::move(node));
        gcdelete(std::move(nodeArray));

        //check
        check(GC::getAllocSize() == prevAllocSize, "Data not deleted correctly");
        check(count == prevCount, "Objects not deleted correctly");

        //collect
        allocSize = GC::collect();

        //check
        check(allocSize == prevAllocSize, "Data not collected correctly");
        check(count == prevCount, "Objects not destroyed correctly");
    });
}


struct Data {
    int value;
};


void scanData(void* mem) {
    Data* data = reinterpret_cast<Data*>(mem);
    data->value = 2;
}


Data* initData(void* mem) {
    ++count;
    Data* data = reinterpret_cast<Data*>(mem);
    data->value = 1;
    return data;
}


void cleanupData(void* mem) {
    --count;
}


void freeData(void* mem) {
    free(mem);
}


void test15() {
    doTest("custom block header", []() {
        size_t prevAllocSize = GC::getAllocSize();
        int prevCount = count;

        //initialize
        GCCustomBlockHeaderVTable vtable{&scanData, &cleanupData, &freeData};        
        GCPtr<Data> data{ gcnew<Data>(sizeof(Data), [](size_t size) { return malloc(size); }, initData, vtable) };

        //check
        check(data->value == 1, "Invalid initialization");
        check(count == prevCount + 1, "Invalid initialization");

        //collect
        GC::collect();

        //check
        check(data->value == 2, "Data not scanned as expected");

        //reset
        data = nullptr;

        //collect
        GC::collect();

        //check
        check(GC::getAllocSize() == prevAllocSize, "Data not collected as expected");
        check(count == prevCount, "Data not collected as expected");
    });
}


struct Node1 : GCIScannableObject {
    GCBasicPtr<Node1> left;
    GCBasicPtr<Node1> right;
    int data;

    Node1(size_t depth = 1, int data = 0) 
        : left(depth > 1 ? gcnew<Node1>(depth - 1) : nullptr)
        , right(depth > 1 ? gcnew<Node1>(depth - 1) : nullptr)
        , data(data)
    {
        if (depth == 0) {
            throw std::invalid_argument("depth");
        }
        count.fetch_add(1, std::memory_order_relaxed);
    }

    ~Node1() {
        count.fetch_sub(1, std::memory_order_relaxed);
    }

    void scan() const noexcept final {
        left.scan();
        right.scan();
    }
};


void test16() {
    doTest("1 thread, 2^20 objects per thread, using basic ptrs", []() {
        //initialize
        int prevCount = count;
        std::thread thread([]() {
            GCPtr<Node1> root = gcnew<Node1>(20);
        });
        thread.join();

        //collect
        const size_t allocSize = GC::collect();

        //wait for collections to finish
        waitCollection(prevCount);
    });
}


void test17() {
    doTest("4 threads, 2^20 objects per thread, using basic ptrs", []() {
        int prevCount = count;

        //initialize
        const size_t ThreadCount = 4;
        const size_t ObjectCountPerThread = 1 << 20;
        std::vector<std::thread> threads;

        for (size_t i = 0; i < ThreadCount; ++i) {
            threads.push_back(std::thread([]() {
                GCPtr<Node1> root = gcnew<Node1>(16);
            }));
        }

        for (std::thread& thread : threads) {
            thread.join();
        }

        //collect
        const size_t allocSize = GC::collect();

        //wait for collections to finish
        waitCollection(prevCount);
    });
}


void test18() {
    doTest("Producer-consumer, 2^20 objects, using basic ptrs", []() {
        int prevCount = count;

        //initialize
        const size_t ObjectCount = 1 << 20;
        std::mutex mutex;
        std::condition_variable cond;
        std::deque<GCPtr<Node1>> objects;

        std::thread consumerThread([&]() {
            size_t receivedObjectCount = 0;
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
            for (size_t i = 0; i < ObjectCount; ++i) {
                {
                    std::lock_guard lock(mutex);
                    objects.push_back(gcnew<Node1>());
                }
                cond.notify_one();
            }
        });

        producerThread.join();
        consumerThread.join();

        //collect
        const size_t allocSize = GC::collect();

        //wait for collections to finish
        waitCollection(prevCount);
    });
}


class SharedObject : public std::enable_shared_from_this<SharedObject> {
public:
    std::shared_ptr<SharedObject> sharedOther;
    GCPtr<SharedObject> gcOther;
};


void test19() {
    doTest("Integration with shared ptrs: shared -> shared", []() {
        size_t initAllocSize = GC::getAllocSize();

        std::shared_ptr<SharedObject> object1{ gcnew<SharedObject>() };
        object1->sharedOther = gcnew<SharedObject>();
        size_t prevAllocSize = GC::getAllocSize();

        //check size without resetting the shared ptr
        size_t allocSize = GC::collect();
        check(allocSize == prevAllocSize, "invalid collection of shareable object");

        //reset the shared ptr; the object must be collected
        object1.reset();
        allocSize = GC::collect();
        check(allocSize == initAllocSize, "invalid deletion of shareable object");
    });
}


void test20() {
    doTest("Integration with shared ptrs: shared -> gc", []() {
        size_t initAllocSize = GC::getAllocSize();

        std::shared_ptr<SharedObject> object1{ gcnew<SharedObject>() };
        object1->gcOther = gcnew<SharedObject>();
        size_t prevAllocSize = GC::getAllocSize();

        //check size without resetting the shared ptr
        size_t allocSize = GC::collect();
        check(allocSize == prevAllocSize, "invalid collection of shareable object");

        //reset the shared ptr; the object must be collected
        object1.reset();
        allocSize = GC::collect();
        check(allocSize == initAllocSize, "invalid deletion of shareable object");
    });
}


void test21() {
    doTest("Integration with shared ptrs: gc -> shared", []() {
        size_t initAllocSize = GC::getAllocSize();

        GCPtr<SharedObject> object1{ gcnew<SharedObject>() };
        object1->sharedOther = gcnew<SharedObject>();
        size_t prevAllocSize = GC::getAllocSize();

        //check size without resetting the shared ptr
        size_t allocSize = GC::collect();
        check(allocSize == prevAllocSize, "invalid collection of gc object");

        //reset the shared ptr; the object must be collected
        object1.reset();
        allocSize = GC::collect();
        check(allocSize == initAllocSize, "invalid deletion of gc object");
    });
}


void test22() {
    doTest("Integration with shared ptrs: object both shared and gc'd", []() {
        size_t initAllocSize = GC::getAllocSize();

        GCPtr<SharedObject> object1{ gcnew<SharedObject>() };
        std::shared_ptr<SharedObject> object2{ object1 };
        size_t prevAllocSize = GC::getAllocSize();

        //check size without resetting any pointers
        size_t allocSize = GC::collect();
        check(allocSize == prevAllocSize, "invalid collection of gc object");

        //reset the gc ptr; the object must not be collected since there is a shared ptr to it
        object1.reset();
        allocSize = GC::collect();
        check(allocSize == prevAllocSize, "invalid collection of gc/shared object after gc ptr reset");

        //reset the shared ptr; the object must be collected since there is no shared ptr to it
        object1 = object2.get();
        object2.reset();
        allocSize = GC::collect();
        check(allocSize == initAllocSize, "invalid collection of gc/shared object after gc/shared ptr reset");
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
    test10();
    test11();
    test12();
    test13();
    test14();
    test15();
    test16();
    test17();
    test18();
    test19();
    test20();
    test21();
    test22();

    if (errorCount > 0) {
        std::cout << "Errors: " << errorCount << std::endl;
    }
    else {
        std::cout << "All tests passed.\n";
    }
    
    system("pause");
    return errorCount;
}
