#include "gclib/VoidPtr.hpp"
#include "Thread.hpp"


namespace gclib
{


    //Assigns a new value in a collector-safe manner.
    VoidPtr& VoidPtr::operator = (void* value) noexcept
    {
        std::lock_guard lockThread(Thread::getInstance().m_mutex);
        m_value = value;
        return *this;
    }


    //Copies a pointer in a collector-safe manner.
    VoidPtr& VoidPtr::operator = (const VoidPtr& ptr) noexcept
    {
        std::lock_guard lockThread(Thread::getInstance().m_mutex);
        m_value = ptr.m_value;
        return *this;
    }


    //Moves a pointer in a collector-safe manner.
    VoidPtr& VoidPtr::operator = (VoidPtr&& ptr) noexcept
    {
        std::lock_guard lockThread(Thread::getInstance().m_mutex);
        void* temp = ptr.m_value;
        ptr.m_value = nullptr;
        m_value = temp;
        return *this;
    }


} //namespace gclib
