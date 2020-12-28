#include "gclib/gcnew.hpp"
#include "gclib/GC.hpp"
#include "GCCollectorData.hpp"


//locks the current thread
GCNewPriv::Lock::Lock() {
    GCThread::instance().mutex.lock();
}


//unlocks the current thread
GCNewPriv::Lock::~Lock() {
    GCThread& thread = GCThread::instance();
    thread.mutex.unlock();
}


//restores the ptr list
GCNewPriv::PtrListOverride::~PtrListOverride() {
    GCThread::instance().ptrs = prevPtrList;
}


//if the allocation limit is exceeded, then collect garbage
void GCNewPriv::collectGarbageIfAllocationLimitIsExceeded() {    
    GCCollectorData& collectorData = GCCollectorData::instance();
    
    //get the current allocation size and limit
    const std::size_t allocSize = collectorData.allocSize.load(std::memory_order_acquire);
    const std::size_t allocLimit = collectorData.allocLimit.load(std::memory_order_acquire);

    //if the allocation size has not yet exceeded the allocation limit, do nothing else
    if (allocSize < allocLimit) {
        return;
    }

    //get the last collection allocation size
    const std::size_t lastCollectionAllocSize = collectorData.lastCollectionAllocSize.load(std::memory_order_acquire);

    //if the current allocation size is smaller than last collection allocation size,
    //then there is no point to initiate a collection, because data are being freed
    if (allocSize <= lastCollectionAllocSize) {
        return;
    }

    //compute the delta between the current alloc size, 
    //which is now greater than the last collection alloc size,
    //and the lat collection alloc size
    const std::size_t allocSizeDelta = allocSize - lastCollectionAllocSize;

    //get the auto collect alloc size delta in order to compare it with the delta computed above
    const std::size_t autoCollectAllocSizeDelta = collectorData.autoCollectAllocSizeDelta.load(std::memory_order_acquire);

    //if the auto collect alloc size delta has not yet been reached, do nothing else;
    //this is in order to avoid continously collecting if the alloc limit is reached
    if (allocSizeDelta <= autoCollectAllocSizeDelta) {
        return;
    }

    //since the alloc size delta has been reached, collect data to free memory
    GC::collectAsync();
}



//allocate gc memory
void* GCNewPriv::allocate(std::size_t size, void(*finalizer)(void*, void*), PtrListOverride& plo) {
    //include block header size
    size += sizeof(GCBlockHeader);

    //allocate block
    GCBlockHeader* block = reinterpret_cast<GCBlockHeader*>(::operator new(size));

    //if no block was allocated, throw exception
    if (!block) {
        throw GCBadAlloc();
    }

    GCThread& thread = GCThread::instance();

    //init the block
    new (block) GCBlockHeader;
    block->end = reinterpret_cast<char*>(block) + size;
    block->finalizer = finalizer;
    block->owner = thread.data;

    //add the block to the thread
    thread.blocks.append(block);

    //override the ptr list
    plo.prevPtrList = thread.ptrs;
    thread.ptrs = &block->ptrs;

    //increment the global allocation size
    GCCollectorData::instance().allocSize.fetch_add(size, std::memory_order_relaxed);

    //return memory after the block
    return block + 1;
}


//deallocate gc memory
void GCNewPriv::deallocate(void* mem) {

    //get the pointer to the block
    GCBlockHeader* block = reinterpret_cast<GCBlockHeader*>(mem) - 1;

    //remove the block from its thread
    block->detach();

    //remove the block's size from the collector
    const std::size_t size = reinterpret_cast<char*>(block->end) - reinterpret_cast<char*>(block);
    GCCollectorData::instance().allocSize.fetch_sub(size, std::memory_order_relaxed);

    //free memory
    ::operator delete(mem);
}

