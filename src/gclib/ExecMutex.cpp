#include <mutex>
#include "ExecMutex.hpp"


namespace gclib {


    //Tries to lock the mutex.; If it fails, then the current thread enters a wait state.
    bool ExecMutex::lock() {
        //if the flag is set, then this thread locked the mutex
        bool prev = false;
        if (m_flag.compare_exchange_strong(prev, true, std::memory_order_acq_rel)) {
            return true;
        }

        //this thread failed to lock the mutex, so wait
        std::mutex mutex;
        std::unique_lock lock(mutex);
        m_waitCondition.wait(lock);

        //failed to enter the execution, so prevent it
        return false;
    }


    //Unlocks the mutex and notifies the rest of the threads.
    void ExecMutex::unlock() {
        m_flag.store(false, std::memory_order_acq_rel);
        m_waitCondition.notify_all();
    }


} //namespace gclib
