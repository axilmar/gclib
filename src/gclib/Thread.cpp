#include "Thread.hpp"


namespace gclib
{


    //the one and only thread instance
    static thread_local Thread thread;


    //returns the current thread instance.
    Thread& Thread::getInstance()
    {
        return thread;
    }


    //constructor; registers the thread to the collector.
    Thread::Thread()
    {
    }


    //destructor; unregisters the thread from the collector; handles collector heap data
    Thread::~Thread()
    {
    }


} //namespace gclib
