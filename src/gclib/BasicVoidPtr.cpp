#include "gclib/BasicVoidPtr.hpp"
#include "ThreadData.hpp"


namespace gclib {


    BasicVoidPtr::BasicVoidPtr(BasicVoidPtr&& ptr) noexcept : m_value(ptr.m_value) {
        std::lock_guard lock(ThreadData::instance().ptrMutex);
        ptr.m_value = nullptr;
    }


    BasicVoidPtr& BasicVoidPtr::operator =(void* value) noexcept {
        std::lock_guard lock(ThreadData::instance().ptrMutex);
        m_value = value;
        return *this;
    }


    BasicVoidPtr& BasicVoidPtr::operator =(const BasicVoidPtr& ptr) noexcept {
        std::lock_guard lock(ThreadData::instance().ptrMutex);
        m_value = ptr.m_value;
        return *this;
    }


    BasicVoidPtr& BasicVoidPtr::operator =(BasicVoidPtr&& ptr) noexcept {
        void* temp = ptr.m_value;
        std::lock_guard lock(ThreadData::instance().ptrMutex);
        ptr.m_value = nullptr;
        m_value = temp;
        return *this;
    }


} //namespace gclib
