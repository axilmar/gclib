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


//returns the block header size
std::size_t GCNewPriv::getBlockHeaderSize() {
    return sizeof(GCBlockHeader);
}


//returns a block's end
void* GCNewPriv::getBlockEnd(const void* start) {
    return reinterpret_cast<const GCBlockHeader*>(start)[-1].end;
}


//register gc memory
void* GCNewPriv::registerAllocation(std::size_t size, void* mem, void(*finalizer)(void*, void*), std::function<void(void*)>&& free, GCList<GCPtrStruct>*& prevPtrList) {
    //get block
    GCBlockHeader* block = reinterpret_cast<GCBlockHeader*>(mem);

    GCThread& thread = GCThread::instance();

    //init the block
    new (block) GCBlockHeader(size, finalizer, std::move(free), thread.data);

    //add the block to the thread
    thread.blocks.append(block);

    //override the ptr list
    prevPtrList = thread.ptrs;
    thread.ptrs = &block->ptrs;

    //increment the global allocation size
    GCCollectorData::instance().allocSize.fetch_add(size, std::memory_order_relaxed);

    //return memory after the block header
    return block + 1;
}


//unregisters an allocation
void GCNewPriv::unregisterAllocation(void* mem) {
    //get the pointer to the block
    GCBlockHeader* block = reinterpret_cast<GCBlockHeader*>(mem);

    //remove the block from its thread
    block->detach();

    //remove the block's size from the collector
    const std::size_t size = reinterpret_cast<char*>(block->end) - reinterpret_cast<char*>(block);
    GCCollectorData::instance().allocSize.fetch_sub(size, std::memory_order_relaxed);
}



//sets the current ptr list
void GCNewPriv::setPtrList(GCList<GCPtrStruct>* ptrList) {
    GCThread::instance().ptrs = ptrList;
}


//deallocate gc memory
void GCNewPriv::deallocate(void* mem) {
    GCBlockHeader* block = reinterpret_cast<GCBlockHeader*>(mem) - 1;
    unregisterAllocation(block);
    block->free(block);
}
