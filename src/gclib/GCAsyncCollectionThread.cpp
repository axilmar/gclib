#include <mutex>
#include "GCAsyncCollectionThread.hpp"
#include "gclib/GC.hpp"


//returns the one and only instance of this class
GCAsyncCollectionThread& GCAsyncCollectionThread::instance() {
    static GCAsyncCollectionThread obj;
    return obj;
}


//starts the collection thread
GCAsyncCollectionThread::GCAsyncCollectionThread() : m_thread([this]() { run(); }) {

}


//stops the collection thread
GCAsyncCollectionThread::~GCAsyncCollectionThread() {
    m_stop.store(true, std::memory_order_release);
    cond.notify_one();
    m_thread.join();
}


//the thread loop
void GCAsyncCollectionThread::run() {
    std::mutex mutex;
    for (;;) {
        std::unique_lock lock(mutex);
        cond.wait(lock);
        if (m_stop.load(std::memory_order_acquire)) return;
        GC::collect();
    }
}
