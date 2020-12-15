#include <iostream>
#include <chrono>
#include <atomic>


template <class F> double time_function(F&& func) {
    auto start = std::chrono::high_resolution_clock::now();
    func();
    auto end = std::chrono::high_resolution_clock::now();
    return std::chrono::duration_cast<std::chrono::duration<double>>(end - start).count();
}


class Ptr {
public:
    Ptr() {
        init();
    }

    Ptr(const Ptr& p) {
        init();
    }

    Ptr(Ptr&& p) {
        init();
    }

    ~Ptr() {
        cleanup();
    }

private:
    Ptr* m_next;
    void init();
    void cleanup();
};


thread_local Ptr* top = nullptr;
thread_local std::atomic<bool> lock;


void Ptr::init() {
    bool prev = false;
    if (lock.compare_exchange_strong(prev, true, std::memory_order_acquire))
    {
        m_next = top;
        top = this;
        lock.store(false, std::memory_order_release);
    }
}


void Ptr::cleanup() {
    bool prev = false;
    if (lock.compare_exchange_strong(prev, true, std::memory_order_acquire))
    {
        top = m_next;
        lock.store(false, std::memory_order_release);
    }
}


Ptr func(int depth) {
    Ptr p;
    if (depth > 1) {
        Ptr p1 = func(depth - 1);
        Ptr p2 = func(depth - 1);
    }
    return p;
}


int main() {
    auto dur = time_function([]() {
        func(24);
    });
    std::cout << dur << std::endl;
    system("pause");
    return 0;
}
