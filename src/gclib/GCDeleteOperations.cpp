#include "gclib/GCDeleteOperations.hpp"
#include "gclib/GCThreadLock.hpp"
#include "GCBlockHeader.hpp"
#include "GCThread.hpp"
#include "GCThread.hpp"
#include "GCCollectorData.hpp"


//unregister block
void GCDeleteOperations::unregisterBlock(GCBlockHeader* block) {
    //remove the block from its thread
    block->detach();

    //remove the block's size from the collector
    const size_t size = reinterpret_cast<char*>(block->end) - reinterpret_cast<char*>(block);
    GCCollectorData::instance().allocSize.fetch_sub(size, std::memory_order_relaxed);
}


//internal block delete
void GCDeleteOperations::deleteBlock(GCBlockHeader* block) {
    //reset the block's pointers so as that the finalizer does not access dangling pointers
    for (GCPtrStruct* ptr = block->ptrs.first(); ptr != block->ptrs.end(); ptr = ptr->next) {
        ptr->mutex = nullptr;
        ptr->value = nullptr;
    }

    //finalize the object or objects
    block->vtable.finalize(block + 1, block->end);

    //free the memory occupied by the block
    block->vtable.free(block);
}


//delete and unregister a block
void GCDeleteOperations::deleteAndUnregisterBlock(class GCBlockHeader* block) {
    {
        GCThreadLock lock;
        unregisterBlock(block);
    }
    deleteBlock(block);
}


//delete operation
void GCDeleteOperations::operatorDelete(void* ptr) {
    if (ptr) {
        deleteAndUnregisterBlock(reinterpret_cast<GCBlockHeader*>(ptr) - 1);
    }
}


//delete block only if has already been collected
void GCDeleteOperations::operatorDeleteIfCollected(void* ptr) {
    if (ptr) {
        GCBlockHeader* block = reinterpret_cast<GCBlockHeader*>(ptr) - 1;
        if (block->collected.load(std::memory_order::memory_order_acquire)) {
            deleteBlock(block);
        }
    }
}
