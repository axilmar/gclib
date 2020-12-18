#ifndef GCLIB_THREAD_HPP
#define GCLIB_THREAD_HPP


#include "Mutex.hpp"
#include "gclib/DList.hpp"
#include "gclib/VoidPtr.hpp"


namespace gclib {


    /**
        Type of generic pointer list.
     */
    using VoidPtrList = DList<VoidPtr>;


    /**
        Data that may persist after the thread terminates.
     */
    class ThreadData : public DNode<ThreadData> {
    public:
        /**
            Mutex used for synchronizing threads and the collector.
         */
        Mutex mutex;

        /**
            list of pointers.
         */
        VoidPtrList ptrs;

        /**
            Returns true if the data are empty.
            @return true if the data are empty.
         */
        bool empty() const noexcept {
            return ptrs.empty();
        }
    };


    /**
        Class that holds the current thread gc data.
     */
    class Thread : public DNode<Thread> {
    public:
        /**
            Data that may persist after the thread terminates.
         */
        ThreadData* data{ new ThreadData };

        /**
            Shortcut to mutex in order to avoid the indirection.
         */
        Mutex& mutex{ data->mutex };

        /**
            Shortcut to pointer list in order to avoid the indirection.
         */
        VoidPtrList &ptrs{ data->ptrs };

        /**
            Mutex used for memory allocation.
         */
        Mutex mallocMutex;

        /**
            Returns the current thread.
            @return the current thread.
         */
        static Thread& instance() noexcept;

        /**
            Adds this thread to the collector.
         */
        Thread();

        /**
            Removes this thread from the collector.
         */
        ~Thread();
    };


} //namespace gclib


#endif //GCLIB_THREAD_HPP
