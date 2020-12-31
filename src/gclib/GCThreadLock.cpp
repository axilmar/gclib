#include "gclib/GCThreadLock.hpp"
#include "GCThread.hpp"


//locks the current thread
GCThreadLock::GCThreadLock() {
    GCThread::instance().mutex.lock();
}


//unlocks the current thread
GCThreadLock::~GCThreadLock() {
    GCThread::instance().mutex.unlock();
}
