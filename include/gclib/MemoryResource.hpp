#ifndef GCLIB_MEMORYRESOURCE_HPP
#define GCLIB_MEMORYRESOURCE_HPP


#include <vector>
#include "gclib/MemoryPool.hpp"


namespace gclib {


    class MemoryResource {
    public:
        MemoryResource(const std::size_t minBlockSize = sizeof(void*), const std::size_t maxBlockSize = 4096, const std::size_t blockIncrement = sizeof(void*), const std::size_t chunkSize = 32);

        MemoryResource(const MemoryResource&) = delete;

        MemoryResource(MemoryResource&& mr);

        MemoryResource& operator =(const MemoryResource&) = delete;

        MemoryResource& operator =(MemoryResource&&) = delete;

        void* allocate(const std::size_t size);

        void deallocate(void* const mem);

    private:
        struct Block {
            MemoryPool* memoryPool;
        };

        std::size_t m_minBlockSize;
        std::size_t m_maxBlockSize;
        std::size_t m_blockIncrement;
        std::vector<MemoryPool> m_memoryPools;

        static void* initBlock(Block* const block, MemoryPool* const mp) noexcept;

        MemoryPool& getMemoryPool(const std::size_t size) noexcept;
    };


} //namespace gclib


#endif //GCLIB_MEMORYRESOURCE_HPP
