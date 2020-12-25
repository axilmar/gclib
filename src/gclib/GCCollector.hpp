#ifndef GCLIB_GCCOLLECTOR_HPP
#define GCLIB_GCCOLLECTOR_HPP


#include <vector>
#include "GCThread.hpp"
#include "GCBlockHeader.hpp"


/**
 * Global data and algorithms.
 */
class GCCollector {
public:
    ///global mutex.
    std::mutex mutex;

    ///list of active threads.
    GCList<GCThreadData> threads;

    ///list of thread data from terminated threads.
    GCList<GCThreadData> terminatedThreads;

    ///current gc cycle.
    std::size_t cycle{ 0 };

    ///all known blocks
    std::vector<GCBlockHeader*> blocks;

    ///Returns the one and only collector instance.
    static GCCollector& instance();
};


#endif //GCLIB_GCCOLLECTOR_HPP
