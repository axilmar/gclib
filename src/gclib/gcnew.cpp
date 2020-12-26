#include "gclib/gcnew.hpp"
#include "gclib/GC.hpp"
#include "GCCollector.hpp"


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
    GCCollector& collector = GCCollector::instance();
    
    //get the current allocation size and limit
    const std::size_t allocSize = collector.allocSize.load(std::memory_order_acquire);
    const std::size_t allocLimit = collector.allocLimit.load(std::memory_order_acquire);

    //if the allocation size has not yet exceeded the allocation limit, do nothing else
    if (allocSize < allocLimit) {
        return;
    }

    //get the last collection allocation size
    const std::size_t lastCollectionAllocSize = collector.lastCollectionAllocSize.load(std::memory_order_acquire);

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
    const std::size_t autoCollectAllocSizeDelta = collector.autoCollectAllocSizeDelta.load(std::memory_order_acquire);

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
    GCCollector::instance().allocSize.fetch_add(size, std::memory_order_relaxed);

    //return memory after the block
    return block + 1;
}
