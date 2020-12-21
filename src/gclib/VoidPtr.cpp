#include "gclib/VoidPtr.hpp"
#include "ThreadData.hpp"


namespace gclib {


    VoidPtr::VoidPtr(void* value) noexcept : BasicVoidPtr(value), m_owner(&ThreadData::instance()) {
        std::lock_guard lock(m_owner->ptrMutex);
        m_owner->ptrs.append(this);
    }


    VoidPtr::VoidPtr(const VoidPtr& ptr) noexcept : BasicVoidPtr(ptr), m_owner(&ThreadData::instance()) {
        std::lock_guard lock(m_owner->ptrMutex);
        m_owner->ptrs.append(this);
    }


    VoidPtr::VoidPtr(VoidPtr&& ptr) noexcept : BasicVoidPtr(ptr.m_value), m_owner(&ThreadData::instance()) {
        std::lock_guard lock(m_owner->ptrMutex);
        m_owner->ptrs.append(this);
        ptr.m_value = nullptr;
    }


    VoidPtr::~VoidPtr() {
        std::lock_guard lock(m_owner->ptrMutex);
        detach();
    }


} //namespace gclib
