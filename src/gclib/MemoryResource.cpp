#include "gclib/MemoryResource.hpp"


namespace gclib {


    MemoryResource::MemoryResource(const std::size_t minBlockSize, const std::size_t maxBlockSize, const std::size_t blockIncrement, const std::size_t chunkSize) 
        : m_minBlockSize(minBlockSize)
        , m_maxBlockSize(maxBlockSize)
        , m_blockIncrement(blockIncrement)
    {
        //check args
        if (minBlockSize == 0) {
            throw std::invalid_argument("invalid minBlockSize");
        }

        if (minBlockSize < blockIncrement) {
            throw std::invalid_argument("minBlockSize less than blockIncrement");
        }

        if ((minBlockSize & (sizeof(void*) - 1)) != 0) {
            throw std::invalid_argument("invalid minBlockSize alignment");
        }

        if (minBlockSize > maxBlockSize) {
            throw std::invalid_argument("minBlockSize greater than maxBlockSize");
        }

        if ((maxBlockSize & (sizeof(void*) - 1)) != 0) {
            throw std::invalid_argument("invalid maxBlockSize alignment");
        }

        if ((blockIncrement & (sizeof(void*) - 1)) != 0) {
            throw std::invalid_argument("invalid blockIncrement alignment");
        }

        //create the memory pools
        const std::size_t memoryPoolCount = (maxBlockSize - minBlockSize) / blockIncrement;
        for (size_t count = 0; count < memoryPoolCount; ++count) {
            m_memoryPools.emplace_back(minBlockSize + count * blockIncrement, chunkSize);
        }
    }


    //move-construct
    MemoryResource::MemoryResource(MemoryResource&& mr)
        : m_minBlockSize(mr.m_minBlockSize)
        , m_maxBlockSize(mr.m_maxBlockSize)
        , m_blockIncrement(mr.m_blockIncrement)
        , m_memoryPools(std::move(mr.m_memoryPools))
    {
    }


    //allocate block
    void* MemoryResource::allocate(const std::size_t size) {
        //round the size
        const std::size_t roundedSize = (size + m_blockIncrement - 1) & ~(m_blockIncrement - 1);

        //if size can be served by memory pool, use one
        if (roundedSize <= m_maxBlockSize) {
            MemoryPool& mp = getMemoryPool(size);
            Block* const block = (Block*)mp.allocate();
            return initBlock(block, &mp);
        }

        //size too large, allocate from the heap
        Block* block = (Block*)::operator new(roundedSize);
        return initBlock(block, nullptr);
    }


    void MemoryResource::deallocate(void* const mem) {
        Block* const block = (Block*)mem - 1;

        //if block was allocated by memory pool, it is deallocated by a memory pool
        if (block->memoryPool) {
            block->memoryPool->deallocate(block);
            return;
        }

        //no memory pool was used; deallocate from the heap
        ::operator delete(block);
    }


    //initializes a block and returns the block memory
    void* MemoryResource::initBlock(Block* const block, MemoryPool* const mp) noexcept {
        block->memoryPool = mp;
        return block + 1;
    }


    //get memory pool from size
    MemoryPool& MemoryResource::getMemoryPool(const std::size_t size) noexcept {
        return m_memoryPools[size / m_blockIncrement];
    }


} //namespace gclib
