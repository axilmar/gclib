#include <algorithm>
#include "gclib/GC.hpp"
#include "GCCollectorData.hpp"
#include "GCAsyncCollectionThread.hpp"


//stops all threads that participate in garbage collection
static bool stopThreads(GCCollectorData& collectorData) {

    //lock the collectorData so as that no new threads can be added during collection;
    //only one thread is allowed to enter collection
    if (!collectorData.mutex.try_lock()) {
        return false;
    }

    //lock existing threads
    for (GCThreadData* data = collectorData.threads.first(); data != collectorData.threads.end(); data = data->next) {
        data->mutex.lock();
    }

    //lock thread data of terminated threads; the thread data might be used from existing threads
    for (GCThreadData* data = collectorData.terminatedThreads.first(); data != collectorData.terminatedThreads.end(); data = data->next) {
        data->mutex.lock();
    }

    //successfully stopped threads
    return true;
}


//resumes all threads that participate in garbage collection
static void resumeThreads(GCCollectorData& collectorData) {

    //unlock thread data of terminated threads
    for (GCThreadData* data = collectorData.terminatedThreads.last(); data != collectorData.terminatedThreads.end(); data = data->prev) {
        data->mutex.unlock();
    }

    //unlock existing threads
    for (GCThreadData* data = collectorData.threads.last(); data != collectorData.threads.end(); data = data->prev) {
        data->mutex.unlock();
    }

    //unlock the collectorData so as that new threads can be added during collection
    collectorData.mutex.unlock();
}


//retrieves all blocks from all threads and thread data, then sorts the blocks by address,
//in order to use binary search for locating blocks
static void gatherAllBlocks(GCCollectorData& collectorData) {

    //find blocks from threads
    for (GCThreadData* data = collectorData.threads.first(); data != collectorData.threads.end(); data = data->next) {
        for (GCBlockHeader* block = data->blocks.first(); block != data->blocks.end(); block = block->next) {
            collectorData.blocks.push_back(block);
        }
    }

    //find blocks from thread data
    for (GCThreadData* data = collectorData.terminatedThreads.first(); data != collectorData.terminatedThreads.end(); data = data->next) {
        for (GCBlockHeader* block = data->blocks.first(); block != data->blocks.end(); block = block->next) {
            collectorData.blocks.push_back(block);
        }
    }

    //sort blocks
    std::sort(collectorData.blocks.begin(), collectorData.blocks.end());
}


//finds a block using binary search
static GCBlockHeader* find(const std::vector<GCBlockHeader*>& blocks, void* addr) {

    //find block with address greater than the given one
    auto it = std::upper_bound(blocks.begin(), blocks.end(), addr, [](void *addr, GCBlockHeader* block) {
        return addr < block;
    });

    //if the result points to the first block, 
    //it means the address points below the lowest heap object,
    //and therefore do nothing
    if (it == blocks.begin()) {
        return nullptr;
    }

    //since the address points to a block lower than the found block,
    //locate the block from the previous element
    GCBlockHeader* block = *(it - 1);

    //if the pointer points inside the block,
    //then return the block
    if (addr >= block + 1 && addr < block->end) {
        return block;
    }

    //not found
    return nullptr;
}


static void scan(GCCollectorData& collectorData, const GCList<GCPtrStruct>& ptrs);


//marks a block as reachable
static void mark(GCCollectorData& collectorData, GCBlockHeader* block) {

    //if the block is already marked, do nothing else
    if (block->cycle == collectorData.cycle) {
        return;
    }

    //set the block cycle in order to mark the block as reachable
    block->cycle = collectorData.cycle;

    //move the block to the list of marked blocks
    //so as that unmarked blocks are easily found later
    block->detach();
    block->owner->markedBlocks.append(block);

    //increment the global allocation size by the size of the marked block
    const std::size_t size = reinterpret_cast<char*>(block->end) - reinterpret_cast<char*>(block);
    collectorData.allocSize.fetch_add(size, std::memory_order_relaxed);

    //scan the member pointers of the block
    scan(collectorData, block->ptrs);
}


//scans a pointer
static void scan(GCCollectorData& collectorData, const GCPtrStruct* ptr) {

    //if null, don't do anything
    if (!ptr->value) {
        return;
    }

    //locate the block the pointer points to
    GCBlockHeader* block = find(collectorData.blocks, ptr->value);

    //if no block is found, do nothing else
    if (!block) {
        return;
    }

    //mark the block as reachable
    mark(collectorData, block);
}


//scans a pointer list
static void scan(GCCollectorData& collectorData, const GCList<GCPtrStruct>& ptrs) {
    for (const GCPtrStruct* ptr = ptrs.first(); ptr != ptrs.end(); ptr = ptr->next) {
        scan(collectorData, ptr);
    }
}


//mark reachable objects
static void mark(GCCollectorData& collectorData) {

    //gather all blocks so as that pointers to middle of blocks are allowed
    gatherAllBlocks(collectorData);

    //if there are no blocks, then do nothing else
    if (collectorData.blocks.empty()) {
        return;
    }

    //next cycle; used for marking reachable blocks
    ++collectorData.cycle;

    //recompute the allocation size as objects are being marked
    collectorData.allocSize.store(0, std::memory_order_relaxed);

    //scan pointers of threads
    for (GCThreadData* data = collectorData.threads.first(); data != collectorData.threads.end(); data = data->next) {
        scan(collectorData, data->ptrs);
    }

    //scan pointers of thread data
    for (GCThreadData* data = collectorData.terminatedThreads.first(); data != collectorData.terminatedThreads.end(); data = data->next) {
        scan(collectorData, data->ptrs);
    }
}


//gathers unreachable blocks/threads; reset all blocks vector
static void cleanup(GCCollectorData& collectorData, GCList<GCBlockHeader>& blocks, GCList<GCThreadData>& threads) {

    //gather unreachable blocks from active threads; 
    //move remaining unmarked objects to the unreachable blocks;
    //move the marked blocks to the blocks
    for (GCThreadData* data = collectorData.threads.first(); data != collectorData.threads.end(); data = data->next) {
        blocks.append(std::move(data->blocks));
        data->blocks = std::move(data->markedBlocks);
    }

    //gather unreachable blocks from terminated threads;
    //also gather empty thread data to delete later
    for (GCThreadData* data = collectorData.terminatedThreads.first(); data != collectorData.terminatedThreads.end();) {

        //if data are empty, put the data into the terminated threads list;
        //unlock its mutex now, since it is to be removed from the list of terminated threads;
        //if that is not done, the mutex will be destroyed while locked
        if (data->empty()) {
            GCThreadData* next = data->next;
            data->mutex.unlock();
            data->detach();
            threads.append(data);
            data = next;
        }

        //else gather the blocks
        else {
            blocks.append(std::move(data->blocks));
            data->blocks = std::move(data->markedBlocks);
            data = data->next;
        }
    }

    //the collectorData blocks are no longer needed; clear them for next collection
    collectorData.blocks.clear();

    //save the current allocation size
    collectorData.lastCollectionAllocSize.store(collectorData.allocSize.load(std::memory_order_acquire), std::memory_order_release);
}


//sweeps a block
static void sweep(GCBlockHeader* block) {

    //reset the block's pointers so as that the finalizer does not access dangling pointers
    for (GCPtrStruct* ptr = block->ptrs.first(); ptr != block->ptrs.end(); ptr = ptr->next) {
        ptr->mutex = nullptr;
        ptr->value = nullptr;
    }

    //finalize
    block->finalize(block + 1, block->end);

    //free memory
    block->free(block);
}


//sweeps unreachable blocks/threads
static void sweep(const GCList<GCBlockHeader>& blocks, const GCList<GCThreadData>& threads) {

    //sweep blocks
    for (GCBlockHeader* block = blocks.first(); block != blocks.end();) {
        GCBlockHeader* next = block->next;
        sweep(block);
        block = next;
    }

    //sweep threads
    for (GCThreadData* data = threads.first(); data != threads.end(); ) {
        GCThreadData* next = data->next;
        delete data;
        data = next;
    }
}


//collect garbage
std::size_t GC::collect() {
    GCCollectorData& collectorData = GCCollectorData::instance();

    //stop threads that are using the collectorData;
    //if the global mutex was not acquired, it means
    //another thread is currently doing collection
    if (!stopThreads(collectorData)) {
        return collectorData.allocSize.load(std::memory_order_acquire);
    }

    //mark reachable blocks
    mark(collectorData);

    //locate unreachable blocks/thread data
    GCList<GCBlockHeader> blocks;
    GCList<GCThreadData> threads;
    cleanup(collectorData, blocks, threads);

    //result
    const std::size_t allocSize = collectorData.allocSize.load(std::memory_order_acquire);

    //resume the previously stopped threads
    resumeThreads(collectorData);

    //delete blocks and threads while the program continues running
    sweep(blocks, threads);

    return allocSize;
}


//Collects data asynchronously. 
void GC::collectAsync() {
    GCAsyncCollectionThread::instance().cond.notify_one();
}



//Returns the current allocation size.
std::size_t GC::getAllocSize() {
    return GCCollectorData::instance().allocSize.load(std::memory_order_acquire);
}


//Returns the current allocation limit.
std::size_t GC::getAllocLimit() {
    return GCCollectorData::instance().allocLimit.load(std::memory_order_acquire);
}


//Sets the current allocation limit.
void setAllocLimit(std::size_t limit) {
    GCCollectorData::instance().allocLimit.store(limit, std::memory_order_release);
}
