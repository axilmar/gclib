#include "gclib/gcnew.hpp"
#include "gclib/GC.hpp"
#include "GCCollectorData.hpp"


//internal register allocation
template <class F> static void* registerAllocationInternal(size_t size, void* mem, GCIBlockHeaderVTable& vtable, GCList<GCPtrStruct>*& prevPtrList, F&& func) {
    //get block
    GCBlockHeader* block = reinterpret_cast<GCBlockHeader*>(mem);

    GCThread& thread = GCThread::instance();

    //init the block
    new (block) GCBlockHeader(size, vtable, thread.data);

    //add the block to the thread
    thread.blocks.append(block);

    //override the ptr list
    prevPtrList = thread.ptrs;
    thread.ptrs = &block->ptrs;

    //increment the global allocation size
    GCCollectorData::instance().allocSize.fetch_add(size, std::memory_order_relaxed);

    //invoke the extra function
    func(thread, block);

    //return memory after the block header
    return block + 1;
}


//if the allocation limit is exceeded, then collect garbage
void GCNewOperations::collectGarbageIfAllocationLimitIsExceeded() {    
    GCCollectorData& collectorData = GCCollectorData::instance();
    
    //get the current allocation size and limit
    const size_t allocSize = collectorData.allocSize.load(std::memory_order_acquire);
    const size_t allocLimit = collectorData.allocLimit.load(std::memory_order_acquire);

    //if the allocation size has not yet exceeded the allocation limit, do nothing else
    if (allocSize < allocLimit) {
        return;
    }

    //get the last collection allocation size
    const size_t lastCollectionAllocSize = collectorData.lastCollectionAllocSize.load(std::memory_order_acquire);

    //if the current allocation size is smaller than last collection allocation size,
    //then there is no point to initiate a collection, because data are being freed
    if (allocSize <= lastCollectionAllocSize) {
        return;
    }

    //compute the delta between the current alloc size, 
    //which is now greater than the last collection alloc size,
    //and the lat collection alloc size
    const size_t allocSizeDelta = allocSize - lastCollectionAllocSize;

    //get the auto collect alloc size delta in order to compare it with the delta computed above
    const size_t autoCollectAllocSizeDelta = collectorData.autoCollectAllocSizeDelta.load(std::memory_order_acquire);

    //if the auto collect alloc size delta has not yet been reached, do nothing else;
    //this is in order to avoid continously collecting if the alloc limit is reached
    if (allocSizeDelta <= autoCollectAllocSizeDelta) {
        return;
    }

    //since the alloc size delta has been reached, collect data to free memory
    GC::collectAsync();
}


//returns the block header size
size_t GCNewOperations::getBlockHeaderSize() {
    return sizeof(GCBlockHeader);
}


//register gc memory
void* GCNewOperations::registerAllocation(size_t size, void* mem, GCIBlockHeaderVTable& vtable, GCList<GCPtrStruct>*& prevPtrList) {
    return registerAllocationInternal(size, mem, vtable, prevPtrList, [](GCThread& thread, GCBlockHeader* block) {});
}


//register gc memory for shareable object
void* GCNewOperations::registerAllocationShared(size_t size, void* mem, GCIBlockHeaderVTable& vtable, GCList<GCPtrStruct>*& prevPtrList, GCISharedScanner& sharedScanner) {
    return registerAllocationInternal(size, mem, vtable, prevPtrList, [&](GCThread& thread, GCBlockHeader* block) {
        thread.data->shareableBlocks.push_back(block);
        block->sharedScanner = &sharedScanner;
    });
}


//sets the current ptr list
void GCNewOperations::setPtrList(GCList<GCPtrStruct>* ptrList) {
    GCThread::instance().ptrs = ptrList;
}


