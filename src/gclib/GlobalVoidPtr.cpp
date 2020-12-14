#include "gclib/GlobalVoidPtr.hpp"
#include "Thread.hpp"


namespace gclib
{


    //The constructor from value.
    GlobalVoidPtr::GlobalVoidPtr(void* value) 
        : VoidPtr(value), m_threadData(Thread::getInstance().m_data)
    {
        std::lock_guard lockThreadDataPtrs(m_threadData->m_ptrMutex);
        m_threadData->m_ptrs.append(this);
    }


    //The copy constructor from void ptr.
    GlobalVoidPtr::GlobalVoidPtr(const VoidPtr& ptr)
        : VoidPtr(ptr), m_threadData(Thread::getInstance().m_data)
    {
        std::lock_guard lockThreadDataPtrs(m_threadData->m_ptrMutex);
        m_threadData->m_ptrs.append(this);
    }


    //The move constructor from void ptr.
    GlobalVoidPtr::GlobalVoidPtr(VoidPtr&& ptr)
        : VoidPtr(ptr), m_threadData(Thread::getInstance().m_data)
    {
        std::lock_guard lockThreadDataPtrs(m_threadData->m_ptrMutex);
        m_threadData->m_ptrs.append(this);
        reset(ptr);
    }


    //The destructor.
    GlobalVoidPtr::~GlobalVoidPtr()
    {
        std::lock_guard lockThreadDataPtrs(m_threadData->m_ptrMutex);
        detach();
    }


} //namespace gclib
