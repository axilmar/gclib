#include "Mutex.hpp"


namespace gclib {


    //Locks the mutex.
    void Mutex::lock() {
        for (;;) {
            //if the lock flag goes atomically from false to true, 
            //then the lock is successfully acquired; otherwise, spin
            bool prev = false;
            if (m_lockFlag.compare_exchange_weak(prev, true, std::memory_order_acquire, std::memory_order_relaxed)) {
                break;
            }

            //if the gc flag is set, go to sleep
            if (m_gcFlag.load(std::memory_order_acquire) == true) {
                std::mutex mutex;
                std::unique_lock lock(mutex);
                m_waitCondition.wait(lock);
            }
        }
    }


    //Unlocks the mutex by setting the internal variable to its previous state.
    void Mutex::unlock() {
        m_lockFlag.store(false, std::memory_order_release);
    }


    //Locks the mutex for collection.
    void Mutex::lockForCollection() {
        //spin in order to lock the mutex; required so as that threads
        //that are trying to lock the mutex do not get a chance to lock
        //the mutex without checking the gc flag
        for (;;) {
            bool prev = false;
            if (m_lockFlag.compare_exchange_weak(prev, true, std::memory_order_acquire, std::memory_order_relaxed)) {
                break;
            }
        }

        //set the gc flag to true in order to put threads to sleep
        m_gcFlag.store(true, std::memory_order_release);
    }


    //Unlocks the mutex for collection.
    void Mutex::unlockForCollection() {
        //set the gc flag to false so as that threads do not fall asleep again
        m_gcFlag.store(false, std::memory_order_release);

        //set the lock flag to false in order to unlock the mutex
        m_lockFlag.store(false, std::memory_order_release);

        //wake up threads that are sleeping on this mutex
        m_waitCondition.notify_all();
    }


} //namespace gclib
