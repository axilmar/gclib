#include "gclib/BasicVoidPtr.hpp"
#include "Thread.hpp"


namespace gclib {


    //Assignment from value.
    BasicVoidPtr& BasicVoidPtr::operator = (void* value) {
        std::lock_guard lock(Thread::thisThread().mutex);
        m_value = value;
        return *this;
    }


    //The copy assignment operator.
    BasicVoidPtr& BasicVoidPtr::operator = (const BasicVoidPtr& ptr) {
        std::lock_guard lock(Thread::thisThread().mutex);
        m_value = ptr.m_value;
        return *this;
    }


    //The move assignment operator.
    BasicVoidPtr& BasicVoidPtr::operator = (BasicVoidPtr&& ptr) {
        std::lock_guard lock(Thread::thisThread().mutex);
        void* temp = ptr.m_value;
        ptr.m_value = nullptr;
        m_value = temp;
        return *this;
    }


} //namespace gclib
