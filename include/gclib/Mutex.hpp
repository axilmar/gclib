#ifndef GCLIB_MUTEX_HPP
#define GCLIB_MUTEX_HPP


#include <atomic>
#include <condition_variable>


namespace gclib {


    /**
        A special mutex class that functions as a spin lock
        unless a thread calls for other threads to wait.
     */
    class Mutex {
    public:
        /**
            Locks this thread by spinning over a variable.
            It blocks if requested by another thread while spinning.
         */
        void lock() noexcept;

        /**
            Unlocks the mutex.
         */
        void unlock() noexcept;

        /**
            Locks the mutex and notifies the other threads to wait.
         */
        void lockNotify() noexcept;

        /**
            Unlocks the mutex and notifies the other threads to wake up.
         */
        void unlockNotify() noexcept;

    private:
        std::atomic<bool> m_lock{ false };
        std::atomic<bool> m_wait{ false };
        std::condition_variable m_waitCondition;
    };


} //namespace gclib


#endif //GCLIB_MUTEX_HPP
