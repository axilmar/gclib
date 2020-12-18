#include "Thread.hpp"


namespace gclib {


    //the one and only thread local object.
    static thread_local Thread thisThread;


    //Returns the current thread.
    Thread& Thread::thisThread() noexcept {
        return gclib::thisThread;
    }


    //Adds this thread to the collector.
    Thread::Thread() {
    }


    //Removes this thread from the collector.
    Thread::~Thread() {
    }


} //namespace gclib
