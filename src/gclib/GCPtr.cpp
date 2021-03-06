#include "gclib/GCPtr.hpp"
#include "GCThread.hpp"


//init ptr, copy source value
void GCPtrPrivate::initCopy(GCPtrStruct* ptr, void* src) {
    GCThread& thread = GCThread::instance();
    ptr->value = src;
    ptr->mutex = &thread.mutex;
    std::lock_guard lock(thread.mutex);
    thread.ptrs->append(ptr);
}


//init ptr, move source value
void GCPtrPrivate::initMove(GCPtrStruct* ptr, void*& src) {
    GCThread& thread = GCThread::instance();
    ptr->value = src;
    ptr->mutex = &thread.mutex;
    std::lock_guard lock(thread.mutex);
    thread.ptrs->append(ptr);
    src = nullptr;
}


//remove ptr from collector
void GCPtrPrivate::cleanup(GCPtrStruct* ptr) {
    if (!ptr->mutex) return;
    std::lock_guard lock(*ptr->mutex);
    ptr->detach();
}
