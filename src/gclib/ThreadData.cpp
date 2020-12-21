#include "ThreadData.hpp"
#include "GlobalData.hpp"


namespace gclib {


    //check if empty
    bool ThreadData::empty() const noexcept {
        return memoryResource.empty();
    }


    //returns the current thread data instance
    ThreadData& ThreadData::instance() {
        static thread_local Thread thread;
        static thread_local ThreadData& td{ *thread.data };
        return td;
    }


    //adds the data to the collector.
    Thread::Thread() {
        GlobalData& cd = GlobalData::instance();
        std::lock_guard lock(cd.mutex);
        cd.activeThreadData.append(&ThreadData::instance());
    }


    //removes the data from the collector or moves them to the terminated data if not empty.
    Thread::~Thread() {
        GlobalData& cd = GlobalData::instance();
        ThreadData& td = ThreadData::instance();
        std::lock_guard lock(cd.mutex);
        td.detach();
        std::lock_guard lockThread(td.memoryMutex);
        if (td.empty()) return;
        cd.terminatedThreadData.append(&td);
    }


} //namespace gclib
