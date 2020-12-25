#ifndef GCLIB_GCTHREAD_HPP
#define GCLIB_GCTHREAD_HPP


#include "gclib/GCPtrStruct.hpp"
#include "gclib/GCList.hpp"
#include "GCBlockHeader.hpp"


/**
 * Per-thread heap-allocated data.
 */
struct GCThreadData : GCNode<GCThreadData> {
    ///the mutex of this thread
    std::recursive_mutex mutex;

    ///the root pointers of this thread.
    GCList<GCPtrStruct> ptrs;

    ///blocks allocated by this thread.
    GCList<GCBlockHeader> blocks;

    ///marked blocks of this this thread.
    GCList<GCBlockHeader> markedBlocks;

    ///checks if the data are empty.
    bool empty() const noexcept {
        return ptrs.empty() && blocks.empty();
    }
};


/**
 * Per-thread thread-local data.
 */
class GCThread {
public:
    ///thread data; might outlive the thread, and therefore allocated on the heap
    GCThreadData* data = new GCThreadData;

    ///mutex shortcut
    std::recursive_mutex& mutex{ data->mutex };

    ///current pointer list; overriden with a block's list when a block is allocated.
    GCList<GCPtrStruct>* ptrs{ &data->ptrs };

    ///block list shortcut
    GCList<GCBlockHeader>& blocks{ data->blocks };

    ///Returns the one and only thread instance for this thread.
    static GCThread& instance();

    ///registers the thread to the collector.
    GCThread();

    ///unregisters the thread from the collector.
    ~GCThread();
};


#endif //GCLIB_GCTHREAD_HPP
