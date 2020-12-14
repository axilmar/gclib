#ifndef GCLIB_THREAD_HPP
#define GCLIB_THREAD_HPP


#include <mutex>
#include "gclib/DoublyLinkedList.hpp"
#include "gclib/GlobalVoidPtr.hpp"
#include "gclib/SinglyLinkedList.hpp"
#include "gclib/StackVoidPtr.hpp"


namespace gclib
{


    //heap allocated thread data
    class ThreadData : public DoublyLinkedListNode<ThreadData>
    {
    public:
        //mutex for global pointers
        std::mutex m_ptrMutex;

        //global ptrs
        DoublyLinkedList<GlobalVoidPtr> m_ptrs;
    };


    //each gc thread has one of these.
    class Thread
    {
    public:
        //mutex for stack-based assigments/constructor/destructors.
        std::mutex m_mutex;

        //stack pointers.
        SinglyLinkedList<StackVoidPtr> m_stackPtrs;

        //heap data
        ThreadData* m_data = new ThreadData();

        //returns the current thread instance.
        static Thread& getInstance();

        //constructor; registers the thread to the collector.
        Thread();

        //destructor; unregisters the thread from the collector; handles collector heap data
        ~Thread();
    };


} //namespace gclib


#endif //GCLIB_THREAD_HPP
