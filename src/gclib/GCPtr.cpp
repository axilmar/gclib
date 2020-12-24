#include "gclib/GCPtr.hpp"
#include "GCThread.hpp"


//init ptr, copy source value
void GCPtrPriv::initCopy(GCPtrStruct* ptr, void* src) {
    GCThread& thread = GCThread::instance();
    ptr->value = src;
    ptr->mutex = &thread.mutex;
    std::lock_guard lock(thread.mutex);
    thread.ptrs->append(ptr);
}


//init ptr, move source value
void GCPtrPriv::initMove(GCPtrStruct* ptr, void*& src) {
    GCThread& thread = GCThread::instance();
    ptr->value = src;
    ptr->mutex = &thread.mutex;
    std::lock_guard lock(thread.mutex);
    thread.ptrs->append(ptr);
    src = nullptr;
}


//remove ptr from collector
void GCPtrPriv::cleanup(GCPtrStruct* ptr) {
    if (!ptr->mutex) return;
    std::lock_guard lock(*ptr->mutex);
    ptr->detach();
}


//copy ptr value.
void GCPtrPriv::copy(void*& dst, void* src) {
    std::lock_guard lock(GCThread::instance().mutex);
    dst = src;
}


//move ptr value.
void GCPtrPriv::move(void*& dst, void*& src) {
    std::lock_guard lock(GCThread::instance().mutex);
    void* temp = src;
    src = nullptr;
    dst = temp;
}
