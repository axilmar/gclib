#ifndef GCLIB_GCASYNCCOLLECTIONTHREAD_HPP
#define GCLIB_GCASYNCCOLLECTIONTHREAD_HPP


#include <thread>
#include <atomic>
#include <condition_variable>


///runs collection in a background thread
class GCAsyncCollectionThread {
public:
    ///condition variable to wait on
    std::condition_variable cond;

    ///returns the one and only instance of this class
    static GCAsyncCollectionThread& instance();

private:
    //stop flag
    std::atomic<bool> m_stop{ false };

    //thread
    std::thread m_thread;

    //starts the collection thread
    GCAsyncCollectionThread();

    //stops the collection thread
    ~GCAsyncCollectionThread();

    //the thread loop
    void run();
};


#endif //GCLIB_GCASYNCCOLLECTIONTHREAD_HPP
