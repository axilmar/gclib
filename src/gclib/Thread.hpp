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
    class ThreadData {
    public:
        /**
            Mutex used for synchronizing threads and the collector.
         */
        Mutex mutex;

        /**
            list of pointers.
         */
        VoidPtrList ptrs;
    };


    /**
        Class that holds the current thread gc data.
     */
    class Thread {
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
            Returns the current thread.
            @return the current thread.
         */
        static Thread& thisThread() noexcept;

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
