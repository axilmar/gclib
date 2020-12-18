#ifndef GCLIB_MUTEX_HPP
#define GCLIB_MUTEX_HPP


#include <atomic>
#include <mutex>
#include <condition_variable>


namespace gclib {


    /**
        Special mutex class used for synchronization.
        It allows putting threads to sleep only if there is a collection in progress.
        Otherwise, it acts as a spinlock.
     */
    class Mutex {
    public:
        /**
            The default constructor.
         */
        Mutex() {
        }

        /**
            The object is not copyable.
         */
        Mutex(const Mutex&) = delete;

        /**
            The object is not movable.
         */
        Mutex(Mutex&&) = delete;

        /**
            The object is not copyable.
         */
        Mutex& operator = (const Mutex&) = delete;

        /**
            The object is not movable.
         */
        Mutex& operator = (Mutex&&) = delete;

        /**
            Locks the mutex.
            It spins over an atomic variable, unless a collection is started.
            Then the current thread goes to sleep.
         */
        void lock();

        /**
            Unlocks the mutex by setting the internal variable to its previous state.
         */
        void unlock();

        /**
            Locks the mutex for collection.
            It waits for the mutex to be available, then it locks it in such a way
            that threads that are trying to lock it using the lock() function are put to sleep.
         */
        void lockForCollection();

        /**
            Unlocks the mutex for collection.
            It notifies all sleeping threads to wake up.
         */
        void unlockForCollection();

    private:
        //lock flag
        std::atomic<bool> m_lockFlag{ false };

        //gc started flag
        std::atomic<bool> m_gcFlag{ false };

        //condition variable to wait upon
        std::condition_variable m_waitCondition;
    };


} //namespace gclib


#endif //GCLIB_MUTEX_HPP
