#include "GCCollector.hpp"


///Returns the one and only thread instance for this thread.
GCThread& GCThread::instance() {
    static thread_local GCThread thread;
    return thread;
}


///registers the thread to the collector.
GCThread::GCThread() {
    GCCollector& collector = GCCollector::instance();
    std::lock_guard lock(collector.mutex);
    collector.threads.append(this);
}


///unregisters the thread from the collector.
GCThread::~GCThread() {
    GCCollector& collector = GCCollector::instance();
    std::lock_guard lock(collector.mutex);
    collector.threads.append(this);
    mutex.lock();
    const bool empty = data->empty();
    mutex.unlock();
    empty ? delete data : collector.threadData.append(data);
}
