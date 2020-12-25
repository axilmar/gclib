#include <algorithm>
#include "gclib/GC.hpp"
#include "GCCollector.hpp"


//stops all threads that participate in garbage collection
static void stopThreads(GCCollector& collector) {

    //lock the collector so as that no new threads can be added during collection
    collector.mutex.lock();

    //lock existing threads
    for (GCThreadData* data = collector.threads.first(); data != collector.threads.end(); data = data->next) {
        data->mutex.lock();
    }

    //lock thread data of terminated threads; the thread data might be used from existing threads
    for (GCThreadData* data = collector.terminatedThreads.first(); data != collector.terminatedThreads.end(); data = data->next) {
        data->mutex.lock();
    }
}


//resumes all threads that participate in garbage collection
static void resumeThreads(GCCollector& collector) {

    //unlock thread data of terminated threads
    for (GCThreadData* data = collector.terminatedThreads.last(); data != collector.terminatedThreads.end(); data = data->prev) {
        data->mutex.unlock();
    }

    //unlock existing threads
    for (GCThreadData* data = collector.threads.last(); data != collector.threads.end(); data = data->prev) {
        data->mutex.unlock();
    }

    //unlock the collector so as that new threads can be added during collection
    collector.mutex.unlock();
}


//retrieves all blocks from all threads and thread data, then sorts the blocks by address,
//in order to use binary search for locating blocks
static void gatherAllBlocks(GCCollector& collector) {

    //find blocks from threads
    for (GCThreadData* data = collector.threads.first(); data != collector.threads.end(); data = data->next) {
        for (GCBlockHeader* block = data->blocks.first(); block != data->blocks.end(); block = block->next) {
            collector.blocks.push_back(block);
        }
    }

    //find blocks from thread data
    for (GCThreadData* data = collector.terminatedThreads.first(); data != collector.terminatedThreads.end(); data = data->next) {
        for (GCBlockHeader* block = data->blocks.first(); block != data->blocks.end(); block = block->next) {
            collector.blocks.push_back(block);
        }
    }

    //sort blocks
    std::sort(collector.blocks.begin(), collector.blocks.end());
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

    //compare the given address with the block range to ensure the pointer
    //does not point between two blocks
    const bool ok = addr >= block + 1 && addr < block->end;

    //if found, return block, else return null
    return ok ? block : nullptr;
}


static void scan(GCCollector& collector, const GCList<GCPtrStruct>& ptrs);


//marks a block as reachable
static void mark(GCCollector& collector, GCBlockHeader* block) {

    //if the block is already marked, do nothing else
    if (block->cycle == collector.cycle) {
        return;
    }

    //set the block cycle in order to mark the block as reachable
    block->cycle = collector.cycle;

    //move the block to the list of marked blocks
    //so as that unmarked blocks are easily found later
    block->detach();
    block->owner->markedBlocks.append(block);

    //scan the member pointers of the block
    scan(collector, block->ptrs);
}


//scans a pointer
static void scan(GCCollector& collector, const GCPtrStruct* ptr) {

    //if null, don't do anything
    if (!ptr->value) {
        return;
    }

    //locate the block the pointer points to
    GCBlockHeader* block = find(collector.blocks, ptr->value);

    //if no block is found, do nothing else
    if (!block) {
        return;
    }

    //mark the block as reachable
    mark(collector, block);
}


//scans a pointer list
static void scan(GCCollector& collector, const GCList<GCPtrStruct>& ptrs) {
    for (const GCPtrStruct* ptr = ptrs.first(); ptr != ptrs.end(); ptr = ptr->next) {
        scan(collector, ptr);
    }
}


//mark reachable objects
static void mark(GCCollector& collector) {

    //gather all blocks so as that pointers to middle of blocks are allowed
    gatherAllBlocks(collector);

    //if there are no blocks, then do nothing else
    if (collector.blocks.empty()) {
        return;
    }

    //next cycle; used for marking reachable blocks
    ++collector.cycle;

    //scan pointers of threads
    for (GCThreadData* data = collector.threads.first(); data != collector.threads.end(); data = data->next) {
        scan(collector, data->ptrs);
    }

    //scan pointers of thread data
    for (GCThreadData* data = collector.terminatedThreads.first(); data != collector.terminatedThreads.end(); data = data->next) {
        scan(collector, data->ptrs);
    }
}


//gathers unreachable blocks/threads; reset all blocks vector
static void cleanup(GCCollector& collector, GCList<GCBlockHeader>& blocks, GCList<GCThreadData>& threads) {

    //gather unreachable blocks from active threads; 
    //move remaining unmarked objects to the unreachable blocks;
    //move the marked blocks to the blocks
    for (GCThreadData* data = collector.threads.first(); data != collector.threads.end(); data = data->next) {
        blocks.append(std::move(data->blocks));
        data->blocks = std::move(data->markedBlocks);
    }

    //gather unreachable blocks from terminated threads;
    //also gather empty thread data to delete later
    for (GCThreadData* data = collector.terminatedThreads.first(); data != collector.terminatedThreads.end();) {

        //if data are empty, gather the data
        if (data->empty()) {
            GCThreadData* next = data->next;
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

    //the collector blocks are no longer needed; clear them for next collection
    collector.blocks.clear();
}


//sweeps a block
static void sweep(GCBlockHeader* block) {

    //reset the block's pointers so as that the finalizer does not access dangling pointers
    for (GCPtrStruct* ptr = block->ptrs.first(); ptr != block->ptrs.end(); ptr = ptr->next) {
        ptr->mutex = nullptr;
        ptr->value = nullptr;
    }

    //finalize
    block->finalizer(block + 1, block->end);

    //free memory
    ::operator delete(block);
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
void GC::collect() {
    GCCollector& collector = GCCollector::instance();

    //stop threads that are using the collector
    stopThreads(collector);

    //mark reachable blocks
    mark(collector);

    //locate unreachable blocks/thread data
    GCList<GCBlockHeader> blocks;
    GCList<GCThreadData> threads;
    cleanup(collector, blocks, threads);

    //resume the previously stopped threads
    resumeThreads(collector);

    //delete blocks and threads while the program continues running
    sweep(blocks, threads);
}
