#include "gclib/StaticVoidPtr.hpp"
#include "Thread.hpp"


namespace gclib {


    //The default constructor.
    StaticVoidPtr::StaticVoidPtr(void* value) {
        m_value = value;
    }


    //The copy constructor.
    StaticVoidPtr::StaticVoidPtr(const StaticVoidPtr& ptr) {
        std::lock_guard lock(Thread::instance().mutex);
        m_value = ptr.m_value;
    }


    //The move constructor.
    StaticVoidPtr::StaticVoidPtr(StaticVoidPtr&& ptr) {
        std::lock_guard lock(Thread::instance().mutex);
        m_value = ptr.m_value;
        ptr.m_value = nullptr;
    }


} //namespace gclib
