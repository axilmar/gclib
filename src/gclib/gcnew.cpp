#include "gclib/gcnew.hpp"
#include "GCThread.hpp"
#include "GCBlockHeader.hpp"


//locks the current thread
GCNew::Lock::Lock() {
    GCThread::instance().mutex.lock();
}


//restores the current pointer list, unlocks the current thread
GCNew::Lock::~Lock() {
    GCThread& thread = GCThread::instance();
    thread.ptrs = reinterpret_cast<decltype(thread.ptrs)>(prevPtrList);
    thread.mutex.unlock();
}


//allocate gc memory
void* GCNew::allocate(std::size_t size, void(*finalizer)(void*, void*), void*&prevPtrList) {
    //include block header size
    size += sizeof(GCBlockHeader);

    //allocate block
    GCBlockHeader* block = reinterpret_cast<GCBlockHeader*>(::operator new(size));

    GCThread& thread = GCThread::instance();

    //init the block
    new (block) GCBlockHeader;
    block->end = reinterpret_cast<char*>(block) + size;
    block->finalizer = finalizer;
    block->mutex = &thread.mutex;

    //add the block to the thread
    thread.blocks.append(block);

    //override the ptr list
    prevPtrList = thread.ptrs;
    thread.ptrs = &block->ptrs;

    //return memory after the block
    return block + 1;
}
