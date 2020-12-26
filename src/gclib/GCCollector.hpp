#ifndef GCLIB_GCCOLLECTOR_HPP
#define GCLIB_GCCOLLECTOR_HPP


#include <vector>
#include <atomic>
#include "GCThread.hpp"
#include "GCBlockHeader.hpp"


/**
 * Global data and algorithms.
 */
class GCCollector {
public:
    ///current allocation size
    std::atomic<std::size_t> allocSize{ 0 };

    ///allocation limit, initially set to 64 MB; 
    ///due to value-based approach, C++ does not need a lot of GC memory
    ///for the majority of cases
    std::atomic<std::size_t> allocLimit{ 64 * 1024 * 1024 };

    ///the last allocation size that caused a collection
    std::atomic<std::size_t> lastCollectionAllocSize{ 0 };

    ///the delta between allocation sizes that must be exceeded in order for an automatic collection to happen
    std::atomic<std::size_t> autoCollectAllocSizeDelta{ 1024 * 1024 };

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
