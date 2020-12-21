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
            m_memoryPools.emplace_back(sizeof(Block) + minBlockSize + count * blockIncrement, chunkSize);
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
            const std::size_t prevAllocSize = mp.allocSize();
            Block* const block = (Block*)mp.allocate();
            m_allocSize += mp.allocSize() - prevAllocSize;
            return initBlock(block, &mp);
        }

        //size too large, allocate from the heap
        const std::size_t totalSize = sizeof(LargeBlock) + roundedSize;
        LargeBlock* block = (LargeBlock*)::operator new(totalSize);
        m_allocSize += totalSize;
        return initBlock(block, totalSize);
    }


    void MemoryResource::deallocate(void* const mem) {
        Block* const block = (Block*)mem - 1;
        MemoryPool* const mp = block->memoryPool;

        //if block was allocated by memory pool, it is deallocated by a memory pool
        if (mp) {
            const std::size_t prevAllocSize = mp->allocSize();
            block->memoryPool->deallocate(block);
            m_allocSize -= prevAllocSize - mp->allocSize();
            return;
        }

        //no memory pool was used; deallocate from the heap
        LargeBlock* lb = (LargeBlock*)mem - 1;
        m_allocSize -= lb->size;
        ::operator delete(lb);
    }


    //initializes a block and returns the block memory
    void* MemoryResource::initBlock(Block* const block, MemoryPool* const mp) noexcept {
        block->memoryPool = mp;
        return block + 1;
    }


    //initializes a large block and returns the block memory
    void* MemoryResource::initBlock(LargeBlock* const block, const std::size_t size) noexcept {
        block->memoryPool = nullptr;
        block->size = size;
        return block + 1;
    }


    //get memory pool from size
    MemoryPool& MemoryResource::getMemoryPool(const std::size_t size) noexcept {
        return m_memoryPools[size / m_blockIncrement];
    }


} //namespace gclib
