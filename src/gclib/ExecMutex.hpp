#ifndef GCLIB_EXECMUTEX_HPP
#define GCLIB_EXECMUTEX_HPP


#include <atomic>
#include <condition_variable>


namespace gclib {


    /**
        Special mutex class that allows the execution of an algorithm
        by the first thread that enters a piece of code.
     */
    class ExecMutex {
    public:
        /**
            Tries to lock the mutex.
            If it fails, then the current thread enters a wait state.
            @return true if locked successfully, false otherwise.
         */
        bool lock();

        /**
            Unlocks the mutex and notifies the rest of the threads
            waiting for this lock to wake up.
         */
        void unlock();

    private:
        std::atomic<bool> m_flag{ false };
        std::condition_variable m_waitCondition;
    };


} //namespace gclib


#endif //GCLIB_EXECMUTEX_HPP
