#include "gclib/make.hpp"
#include "ThreadData.hpp"
#include "Block.hpp"


namespace gclib {


    VoidPtr Make::allocate(const std::size_t size, IObjectManager* om) {
        ThreadData& td = ThreadData::instance();
        std::lock_guard lock(td.memoryMutex);
        Block* block = (Block*)td.memoryResource.allocate(sizeof(Block) + size);
        block->objectManager = om;
        return block + 1;
    }


    void Make::deallocate(void* const mem) {
        ThreadData& td = ThreadData::instance();
        std::lock_guard lock(td.memoryMutex);
        Block* block = (Block*)mem - 1;
        td.memoryResource.deallocate(block);
    }


} //namespace gclib
