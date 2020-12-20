#ifndef GCLIB_MEMORYPOOL_HPP
#define GCLIB_MEMORYPOOL_HPP


#include "DList.hpp"


namespace gclib {


    /**
        A memory pool.
        @param BlockSize number of bytes per block.
        @param ChunkSize number of blocks per chunk.
     */
    template <std::size_t BlockSize, std::size_t ChunkSize = 32> class MemoryPool {
    public:
        static_assert(BlockSize > 0, "BlockSize should be greater than 0");
        static_assert((BlockSize & (sizeof(void*) - 1)) == 0, "BlockSize should have pointer alignment");
        static_assert(ChunkSize > 0, "ChunkSize should be greater than 0");

        /**
            The default constructor.
         */
        MemoryPool() {
        }

        /**
            The object cannot be copied.
         */
        MemoryPool(const MemoryPool&) = delete;

        /**
            The object cannot be moved.
         */
        MemoryPool(MemoryPool&&) = delete;

        /**
            The destructor.
            It does not delete any objects or free any memory blocks.
         */
        ~MemoryPool() {

        }

        /**
            The object cannot be copied.
         */
        MemoryPool& operator =(const MemoryPool&) = delete;

        /**
            The object cannot be moved.
         */
        MemoryPool& operator =(MemoryPool&&) = delete;

        /**
            Allocates memory for a block.
            Allocation complexity is O(1): if there is a chunk with at least one available block,
            then that block is used, otherwise a new chunk is created from the c++ heap.
            @return pointer to allocated block.
         */
        void* allocate() {
            //if there are available chunks
            if (m_availableChunks.notEmpty()) {

                //get the last chunk
                Chunk* const chunk = m_availableChunks.last();

                //take a free block from the chunk if there is one
                if (chunk->free < (char*)(chunk + 1) + (sizeof(Block) + BlockSize) * ChunkSize) {
                    Block* const block = getFreeBlock(chunk);
                    return block + 1;
                }

                //take a deleted block
                Block* const block = getDeletedBlock(chunk);
                return block + 1;
            }

            //else if there are no available chunks, create a new one 
            Chunk* const chunk = createChunk();
            Block* const block = getFreeBlock(chunk);
            return block + 1;
        }

        /**
            Frees a memory allocated by this pool.
            Deallocation complexity is O(1): the block is simply added
            to the list of freed blocks of its chunk.
            If the chunk has no more allocated blocks, it is deleted
            and memory is returned to the c++ heap.
            @param mem pointer to memory as returned by 'allocate'.
         */
        void deallocate(void* const mem) {
            Block* const block = (Block*)mem - 1;
            Chunk* const chunk = block->chunk;
            const std::size_t newAllocatedBlockCount = --chunk->allocatedBlockCount;

            //if block count didn't reach 0, then put block in deleted blocks
            if (newAllocatedBlockCount > 0) {
                chunk->deletedBlocks.append(block);

                //if this is the first block to delete,
                //put the chunk back to the allocated chunks
                if (newAllocatedBlockCount == ChunkSize - 1) {
                    m_availableChunks.append(chunk);
                }
            }

            //else if block count reached 0, then delete the chunk
            else {
                chunk->detach();
                ::operator delete(chunk);
            }
        }

    private:
        struct Chunk;

        //block header
        struct Block : DNode<Block> {
            Chunk* chunk;
        };

        //chunk header
        struct Chunk : DNode<Chunk> {
            DList<Block> deletedBlocks;
            std::size_t allocatedBlockCount{ 0 };
            char* free;
        };

        //list of chunks with some free blocks
        DList<Chunk> m_availableChunks;

        //adds a block to the allocated list;
        //if the chunk size is reached, the chunk becomes no longer available
        void addAllocatedBlock(Chunk* chunk, Block* block) {
            ++chunk->allocatedBlockCount;
            if (chunk->allocatedBlockCount == ChunkSize) {
                chunk->detach();
            }
        }

        //returns a block from the deleted list
        Block* getDeletedBlock(Chunk* chunk) {
            Block* block = chunk->deletedBlocks.last();
            block->detach();
            addAllocatedBlock(chunk, block);
            return block;
        }

        //returns a block from the free list
        Block* getFreeBlock(Chunk* chunk) {
            Block* block = (Block*)chunk->free;
            block->chunk = chunk;
            chunk->free += sizeof(Block) + BlockSize;
            addAllocatedBlock(chunk, block);
            return block;
        }

        //creates a chunk
        Chunk* createChunk() {
            Chunk* chunk = (Chunk*)::operator new(sizeof(Chunk) + (sizeof(Block) + BlockSize) * ChunkSize);
            new (chunk) Chunk;
            chunk->free = (char*)(chunk + 1);
            m_availableChunks.append(chunk);
            return chunk;
        }
    };


} //namespace gclib


#endif //GCLIB_MEMORYPOOL_HPP
