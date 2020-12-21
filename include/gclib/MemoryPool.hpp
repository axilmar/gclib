#ifndef GCLIB_MEMORYPOOL_HPP
#define GCLIB_MEMORYPOOL_HPP


#include "DList.hpp"


namespace gclib {


    /**
        A memory pool.
        It allows the quick allocation of memory blocks of a specific size.
     */
    class MemoryPool {
    public:
        /**
            Constructor.
            @param blockSize block size.
            @param chunkSize number of blocks per chunk to preallocate.
            @exception std::invalid_argument thrown if block size is 0 or not alligned to ptr alignment,
                or if chunk size is 0.
         */
        MemoryPool(const std::size_t blockSize, const std::size_t chunkSize = 16);

        /**
            The object cannot be copied.
         */
        MemoryPool(const MemoryPool&) = delete;

        /**
            Moves the given memory pool to this.
            @param mp source memory pool.
         */
        MemoryPool(MemoryPool&& mp);

        /**
            Checks if the pool is empty. If not so, then throws an exception.
            @exception std::runtime_error thrown if the pool is not empty.
         */
        ~MemoryPool();

        /**
            The object cannot be copied.
         */
        MemoryPool& operator =(const MemoryPool&) = delete;

        /**
            The object cannot be moved after construction.
         */
        MemoryPool& operator =(MemoryPool&&) = delete;

        /**
            Allocates memory for a block.
            Allocation complexity is O(1): if there is a chunk with at least one available block,
            then that block is used, otherwise a new chunk is created from the c++ heap.
            @return pointer to allocated block.
         */
        void* allocate();

        /**
            Frees a memory allocated by this pool.
            Deallocation complexity is O(1): the block is simply added
            to the list of freed blocks of its chunk.
            If the chunk has no more allocated blocks, it is deleted
            and memory is returned to the c++ heap.
            @param mem pointer to memory as returned by 'allocate'.
         */
        void deallocate(void* const mem);

        /**
            Applies a function to each allocated block.
            @param func function to use for visiting allocated blocks.
         */
        template <class F> void forEach(F&& func) const {
            m_fullChunks.forEach([&](Chunk* chunk) {
                chunk->allocatedBlocks.forEach([&](Block* block) {
                    func(block + 1);
                });
            });
            m_availableChunks.forEach([&](Chunk* chunk) {
                chunk->allocatedBlocks.forEach([&](Block* block) {
                    func(block + 1);
                });
            });
        }

    private:
        struct Block;
        struct Chunk;

        //block header
        struct Block : DNode<Block> {
            Chunk* chunk;
        };

        //chunk header
        struct Chunk : DNode<Chunk> {
            DList<Block> allocatedBlocks;
            std::size_t allocatedBlockCount{ 0 };
            DList<Block> deletedBlocks;
            char* free;
            char* end;
        };

        //block size
        std::size_t m_blockSize;

        //chunk size
        std::size_t m_chunkSize;

        //block area size
        std::size_t m_blockAreaSize;

        //list of chunks that are fully allocated
        DList<Chunk> m_fullChunks;

        //list of chunks with some free blocks
        DList<Chunk> m_availableChunks;

        //adds a block to the allocated list;
        //if the chunk size is reached, the chunk becomes no longer available
        void addAllocatedBlock(Chunk* chunk, Block* block) noexcept;

        //returns a block from the deleted list
        Block* getDeletedBlock(Chunk* chunk) noexcept;

        //returns a block from the free list
        Block* getFreeBlock(Chunk* chunk) noexcept;

        //computes the block area size
        std::size_t computeBlockAreaSize() noexcept;

        //creates a chunk
        Chunk* createChunk();
    };


} //namespace gclib


#endif //GCLIB_MEMORYPOOL_HPP
