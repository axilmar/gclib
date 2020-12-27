#include "GCCollectorData.hpp"


///Returns the one and only thread instance for this thread.
GCThread& GCThread::instance() {
    static thread_local GCThread thread;
    return thread;
}


///registers the thread to the collector.
GCThread::GCThread() {
    GCCollectorData& collectorData = GCCollectorData::instance();
    std::lock_guard lock(collectorData.mutex);
    collectorData.threads.append(data);
}


///unregisters the thread from the collector.
GCThread::~GCThread() {
    GCCollectorData& collectorData = GCCollectorData::instance();
    std::lock_guard lock(collectorData.mutex);
    data->detach();
    mutex.lock();
    const bool empty = data->empty();
    mutex.unlock();
    empty ? delete data : collectorData.terminatedThreads.append(data);
}
