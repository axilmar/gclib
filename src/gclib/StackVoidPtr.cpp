#include "gclib/StackVoidPtr.hpp"
#include "Thread.hpp"


namespace gclib
{ 


    //Constructor from value.
    StackVoidPtr::StackVoidPtr(void* value) : VoidPtr(value)
    {
        Thread& thread = Thread::getInstance();
        std::lock_guard lockThread(thread.m_mutex);
        thread.m_stackPtrs.prepend(this);
    }


    //Copy constructor from void ptr.
    StackVoidPtr::StackVoidPtr(const VoidPtr& ptr) : VoidPtr(ptr)
    {
        Thread& thread = Thread::getInstance();
        std::lock_guard lockThread(thread.m_mutex);
        thread.m_stackPtrs.prepend(this);
    }


    //Move constructor from void ptr.
    StackVoidPtr::StackVoidPtr(VoidPtr&& ptr) : VoidPtr(ptr)
    {
        Thread& thread = Thread::getInstance();
        std::lock_guard lockThread(thread.m_mutex);
        thread.m_stackPtrs.prepend(this);
        reset(ptr);
    }


    //The destructor.
    StackVoidPtr::~StackVoidPtr()
    {
        Thread& thread = Thread::getInstance();
        std::lock_guard lockThread(thread.m_mutex);
        thread.m_stackPtrs.remove(this);
    }


} //namespace gclib
