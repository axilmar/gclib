#include "Thread.hpp"
#include "Collector.hpp"


namespace gclib {


    //the one and only thread local object.
    static thread_local Thread instance;


    //Returns the current thread.
    Thread& Thread::instance() noexcept {
        return gclib::instance;
    }


    //Adds this thread to the collector.
    Thread::Thread() {
        Collector& collector = Collector::instance();
        std::lock_guard lock(collector.mutex);
        collector.threads.append(this);
    }


    //Removes this thread from the collector.
    Thread::~Thread() {
        Collector& collector = Collector::instance();
        std::lock_guard lock(collector.mutex);
        detach();
        data->empty() ?  delete data : collector.threadData.append(data);
    }


} //namespace gclib
