#include <mutex>
#include "gclib/Mutex.hpp"


namespace gclib {


    //Locks this thread by spinning over a variable.
    void Mutex::lock() {
        std::mutex waitMutex;

        for (;;) {
            if (m_lock.exchange(true, std::memory_order_acquire) == false) {
                break;
            }            
            while (m_lock.load(std::memory_order_relaxed)) {
                if (m_wait.load(std::memory_order_acquire)) {
                    std::unique_lock lock(waitMutex);
                    m_waitCondition.wait(lock);
                }

                else {
                    std::this_thread::yield();
                }
            }
        }
    }


    //Unlocks the mutex.
    void Mutex::unlock() {
        m_lock.store(false, std::memory_order_release);
    }


    //Locks the mutex and notifies the other threads to sleep.
    void Mutex::lockNotify() {
        std::mutex waitMutex;

        //try to lock the mutex
        for (;;) {
            if (m_lock.exchange(true, std::memory_order_acquire) == false) {
                break;
            }
            while (m_lock.load(std::memory_order_relaxed)) {
                std::this_thread::yield();
            }
        }

        //try to acquire the wait flag so as that other threads are blocked
        for (;;) {
            if (m_wait.exchange(true, std::memory_order_acquire) == false) {
                break;
            }
            while (m_wait.load(std::memory_order_relaxed)) {
                std::this_thread::yield();
            }
        }
    }


    //Unlocks the mutex and notifies the other threads to wake up.
    void Mutex::unlockNotify() {
        m_wait.store(false, std::memory_order_release);
        m_lock.store(false, std::memory_order_release);
        m_waitCondition.notify_all();
    }


} //namespace gclib
