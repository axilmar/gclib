#include <stdexcept>
#include "gclib/MemoryPool.hpp"


namespace gclib {


    //Constructor.
    MemoryPool::MemoryPool(const std::size_t blockSize, const std::size_t chunkSize) 
        : m_blockSize(blockSize)
        , m_chunkSize(chunkSize)
        , m_blockAreaSize(computeBlockAreaSize())
    {
        if (blockSize == 0) {
            throw std::invalid_argument("invalid blockSize");
        }

        if ((blockSize & (sizeof(void*) - 1)) != 0) {
            throw std::invalid_argument("invalid blockSize alignment");
        }

        if (chunkSize == 0) {
            throw std::invalid_argument("invalid chunkSize");
        }
    }


    //Moves the given memory pool to this.
    MemoryPool::MemoryPool(MemoryPool&& mp)
        : m_blockSize(mp.m_blockSize)
        , m_chunkSize(mp.m_chunkSize)
        , m_blockAreaSize(mp.m_blockAreaSize)
        , m_availableChunks(std::move(mp.m_availableChunks))
    {
    }


    //Allocates memory for a block.
    void* MemoryPool::allocate() {
        //if there are available chunks
        if (m_availableChunks.notEmpty()) {
            //get the last chunk
            Chunk* const chunk = m_availableChunks.last();

            //take a free block from the chunk if there is one
            if (chunk->free < chunk->end) {
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


    //Frees a memory allocated by this pool.
    void MemoryPool::deallocate(void* const mem) {
        Block* const block = (Block*)mem - 1;
        Chunk* const chunk = block->chunk;

        //remove block
        block->detach();
        const std::size_t newAllocatedBlockCount = --chunk->allocatedBlockCount;

        //if block count didn't reach 0, then put block in deleted blocks
        if (newAllocatedBlockCount > 0) {
            chunk->deletedBlocks.append(block);

            //if this is the first block to delete,
            //put the chunk back to the available chunks
            if (newAllocatedBlockCount == m_chunkSize - 1) {
                chunk->detach();
                m_availableChunks.prepend(chunk);
            }
        }

        //else if block count reached 0, then delete the chunk
        else {
            chunk->detach();
            ::operator delete(chunk);
        }
    }


    //adds a block to the allocated list;
    //if the chunk size is reached, the chunk becomes no longer available
    void MemoryPool::addAllocatedBlock(Chunk* chunk, Block* block) noexcept {
        //add block
        chunk->availableBlocks.append(block);
        ++chunk->allocatedBlockCount;

        //if the chunk is full, move it to the full chunks
        if (chunk->allocatedBlockCount == m_chunkSize) {
            chunk->detach();
            m_fullChunks.append(chunk);
        }
    }


    //returns a block from the deleted list
    MemoryPool::Block* MemoryPool::getDeletedBlock(Chunk* chunk) noexcept {
        //get a previously deleted block
        Block* block = chunk->deletedBlocks.last();
        block->detach();

        //put the block in the allocated list
        addAllocatedBlock(chunk, block);
        return block;
    }


    //returns a block from the free list
    MemoryPool::Block* MemoryPool::getFreeBlock(Chunk* chunk) noexcept {
        //get a free chunk
        Block* block = (Block*)chunk->free;
        block->chunk = chunk;
        chunk->free += sizeof(Block) + m_blockSize;

        //put the block in the allocated list
        addAllocatedBlock(chunk, block);
        return block;
    }


    //computes the block area size
    std::size_t MemoryPool::computeBlockAreaSize() noexcept {
        return (sizeof(Block) + m_blockSize) * m_chunkSize;
    }


    //creates a chunk
    MemoryPool::Chunk* MemoryPool::createChunk() {
        //allocate the chunk from the heap
        Chunk* chunk = (Chunk*)::operator new(sizeof(Chunk) + m_blockAreaSize);

        //initialize the chunk
        new (chunk) Chunk;
        chunk->free = (char*)(chunk + 1);
        chunk->end = chunk->free + m_blockAreaSize;

        //put the chunk in the available chunks
        m_availableChunks.append(chunk);
        return chunk;
    }


} //namespace gclib
