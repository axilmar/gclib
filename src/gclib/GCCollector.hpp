#ifndef GCLIB_GCCOLLECTOR_HPP
#define GCLIB_GCCOLLECTOR_HPP


#include "GCThread.hpp"


/**
 * Global data and algorithms.
 */
class GCCollector {
public:
    ///global mutex.
    std::mutex mutex;

    ///list of active threads.
    GCList<GCThread> threads;

    ///list of thread data from terminated threads.
    GCList<GCThreadData> threadData;

    ///Returns the one and only collector instance.
    static GCCollector& instance();
};


#endif //GCLIB_GCCOLLECTOR_HPP
