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
            Checks if it is empty.
            @return true if empty, false otherwise.
         */
        bool empty() const noexcept {
            return m_allocSize == 0;
        }

        /**
            Returns the allocated size.
            @return the allocated size.
         */
        std::size_t allocSize() const noexcept {
            return m_allocSize;
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

        struct LargeBlock : Block {
            std::size_t size;
        };

        std::size_t m_minBlockSize;
        std::size_t m_maxBlockSize;
        std::size_t m_blockIncrement;
        std::vector<MemoryPool> m_memoryPools;
        std::size_t m_allocSize{ 0 };

        //initializes a block and returns the block memory
        static void* initBlock(Block* const block, MemoryPool* const mp) noexcept;

        //initializes a large block and returns the block memory
        static void* initBlock(LargeBlock* const block, const std::size_t size) noexcept;

        //get memory pool from size
        MemoryPool& getMemoryPool(const std::size_t size) noexcept;
    };


} //namespace gclib


#endif //GCLIB_MEMORYRESOURCE_HPP
