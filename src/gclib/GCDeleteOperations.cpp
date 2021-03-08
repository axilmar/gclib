#include "gclib/GCDeleteOperations.hpp"
#include "gclib/GCThreadLock.hpp"
#include "GCBlockHeader.hpp"
#include "GCThread.hpp"
#include "GCThread.hpp"
#include "GCCollectorData.hpp"


//unregisters an allocation
void GCDeleteOperations::unregisterAllocation(void* mem) {
    //get the pointer to the block
    GCBlockHeader* block = reinterpret_cast<GCBlockHeader*>(mem);

    //remove the block from its thread
    block->detach();

    //remove the block's size from the collector
    const size_t size = reinterpret_cast<char*>(block->end) - reinterpret_cast<char*>(block);
    GCCollectorData::instance().allocSize.fetch_sub(size, std::memory_order_relaxed);
}


//delete operation
void GCDeleteOperations::gcdelete(void* ptr) {
    //if the pointer is null, do nothing else
    if (!ptr) {
        return;
    }

    GCBlockHeader* block = reinterpret_cast<GCBlockHeader*>(ptr) - 1;

    //finalize the object or objects
    block->vtable.finalize(block + 1, block->end);

    //remove the block from the collector
    GCThreadLock lock;
    unregisterAllocation(block);
    block->vtable.free(block);
}
