#ifndef GCLIB_MEMORYRESOURCE_HPP
#define GCLIB_MEMORYRESOURCE_HPP


#include <vector>
#include "gclib/MemoryPool.hpp"


namespace gclib {


    /**
        A memory resource.
        It does not attempt to take onwership of memory blocks,
        because those might contain objects that would have to be deleted first.
     */
    class MemoryResource {
    public:
        /**
            Constructor.
            @param minBlockSize minimum block size for memory pools.
            @param maxBlockSize maximum block size for memory pools.
            @param blockIncrement size difference between two successive memory pools.
            @param chunkSize memory pool chunk size.
            @exception std::invalid_argument thrown if one of the above parameters is incorrect (incorrect value or incorrect alignment).
         */
        MemoryResource(const std::size_t minBlockSize = sizeof(void*), const std::size_t maxBlockSize = 4096, const std::size_t blockIncrement = sizeof(void*), const std::size_t chunkSize = 16);

        /**
            Attempts to delete all memory pools.
            @exception std::runtime_error thrown if any of the memory pools is not empty.
         */
        ~MemoryResource() {
        }

        /**
            The object is not copyable.
         */
        MemoryResource(const MemoryResource&) = delete;

        /**
            Moves the given memory resource to this.
            @param mr source object.
         */
        MemoryResource(MemoryResource&& mr);

        /**
            The object is not copyable.
         */
        MemoryResource& operator =(const MemoryResource&) = delete;

        /**
            The object is not move-assignable.
         */
        MemoryResource& operator =(MemoryResource&&) = delete;

        /**
            Returns the array of memory pools used internally.
            @return the array of memory pools used internally.
         */
        std::vector<MemoryPool>& memoryPools() noexcept {
            return m_memoryPools;
        }

        /**
            Allocates a memory block of the given size.
            @param size number of bytes to allocate; if it exceeds MaxBlockSize, then it is allocated from the heap,
                else it is allocated from a memory pool.
            @return pointer to allocated block.
         */
        void* allocate(const std::size_t size);

        /**
            Deallocates a memory block.
            @param mem pointer to memory to deallocate, as returned by allocate().
         */
        void deallocate(void* const mem);

    private:
        struct Block {
            MemoryPool* memoryPool;
        };

        std::size_t m_minBlockSize;
        std::size_t m_maxBlockSize;
        std::size_t m_blockIncrement;
        std::vector<MemoryPool> m_memoryPools;

        //initializes a block and returns the block memory
        static void* initBlock(Block* const block, MemoryPool* const mp) noexcept;

        //get memory pool from size
        MemoryPool& getMemoryPool(const std::size_t size) noexcept;
    };


} //namespace gclib


#endif //GCLIB_MEMORYRESOURCE_HPP
