#include "gclib/VoidPtr.hpp"
#include "Thread.hpp"


namespace gclib {


    //The default constructor.
    VoidPtr::VoidPtr(void* value) : m_mutex(Thread::thisThread().mutex) {
        m_value = value;
        std::lock_guard lock(m_mutex);
        Thread::thisThread().ptrs.append(this);
    }


    //The copy constructor.
    VoidPtr::VoidPtr(const VoidPtr& ptr) : m_mutex(Thread::thisThread().mutex) {
        std::lock_guard lock(m_mutex);
        m_value = ptr.m_value;
        Thread::thisThread().ptrs.append(this);
    }


    //The move constructor.
    VoidPtr::VoidPtr(VoidPtr&& ptr) : m_mutex(Thread::thisThread().mutex) {
        std::lock_guard lock(m_mutex);
        m_value = ptr.m_value;
        ptr.m_value = nullptr;
        Thread::thisThread().ptrs.append(this);
    }


    //The destructor.
    VoidPtr::~VoidPtr() {
        std::lock_guard lock(m_mutex);
        detach();
    }


    //Assignment from value.
    VoidPtr& VoidPtr::operator = (void* value) {
        BasicVoidPtr::operator = (value);
        return *this;
    }


    //The copy assignment operator.
    VoidPtr& VoidPtr::operator = (const VoidPtr& ptr) {
        BasicVoidPtr::operator = (ptr);
        return *this;
    }


    //The move assignment operator.
    VoidPtr& VoidPtr::operator = (VoidPtr&& ptr) {
        BasicVoidPtr::operator = (ptr);
        return *this;
    }


} //namespace gclib
