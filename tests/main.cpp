#include <iostream>
#include <chrono>
#include <vector>
#include <memory_resource>
#include "gclib/DList.hpp"


using namespace gclib;


template <class F> double timeFunction(F&& func) {
    auto start = std::chrono::high_resolution_clock::now();
    func();
    auto end = std::chrono::high_resolution_clock::now();
    return std::chrono::duration_cast<std::chrono::duration<double>>(end - start).count();
}


constexpr std::size_t MAX_OBJECTS = 5'000'000;


class Object {
public:
    char data[64];
};
std::vector<Object*> data(MAX_OBJECTS);


void test1() {
    for (std::size_t i = 0; i < MAX_OBJECTS; ++i) {
        data[i] = new Object;
    }
    for (std::size_t i = 0; i < MAX_OBJECTS; ++i) {
        delete data[i];
    }
}


class Object2 {
public:
    char data[64];
    void* operator new(std::size_t size);
    void operator delete(void* mem);
};
std::vector<Object2*> data2(MAX_OBJECTS);
std::pmr::unsynchronized_pool_resource memPool;


void* Object2::operator new(std::size_t size) {
    return memPool.allocate(size);
}


void Object2::operator delete(void* mem) {
    return memPool.deallocate(mem, sizeof(Object2));
}


void test2() {
    for (std::size_t i = 0; i < MAX_OBJECTS; ++i) {
        data2[i] = new Object2;
    }
    for (std::size_t i = 0; i < MAX_OBJECTS; ++i) {
        delete data2[i];
    }
}


class Object3 {
public:
    char data[64];
    void* operator new(std::size_t size);
    void operator delete(void* mem);
};
std::vector<Object3*> data3(MAX_OBJECTS);


template <std::size_t BlockSize, std::size_t ChunkSize = 32> class ObjectMemoryPool {
public:
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

    void deallocate(void* const mem) {
        Block* const block = (Block*)mem - 1;
        Chunk* const chunk = block->chunk;
        const std::size_t newAllocatedBlockCount = --chunk->allocatedBlockCount;

        //if block count didn't reach 0, then put block in deleted blocks
        if (newAllocatedBlockCount > 0) {
            block->detach();
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

    struct Block : DNode<Block> {
        Chunk* chunk;
    };

    struct Chunk : DNode<Chunk> {
        DList<Block> allocatedBlocks;
        DList<Block> deletedBlocks;
        std::size_t allocatedBlockCount{0};
        char* free;
    };

    DList<Chunk> m_availableChunks;

    void addAllocatedBlock(Chunk* chunk, Block* block) {
        chunk->allocatedBlocks.append(block);
        ++chunk->allocatedBlockCount;
        if (chunk->allocatedBlockCount == ChunkSize) {
            chunk->detach();
        }
    }

    Block* getDeletedBlock(Chunk* chunk) {
        Block* block = chunk->deletedBlocks.last();
        block->detach();
        addAllocatedBlock(chunk, block);
        return block;
    }

    Block* getFreeBlock(Chunk* chunk) {
        Block* block = (Block*)chunk->free;
        block->chunk = chunk;
        chunk->free += sizeof(Block) + BlockSize;
        addAllocatedBlock(chunk, block);
        return block;
    }

    Chunk* createChunk() {
        Chunk* chunk = (Chunk*)::operator new(sizeof(Chunk) + (sizeof(Block) + BlockSize) * ChunkSize);
        new (chunk) Chunk;
        chunk->free = (char*)(chunk + 1);
        m_availableChunks.append(chunk);
        return chunk;
    }
};


ObjectMemoryPool<sizeof(Object3)> memPool3;


void* Object3::operator new(std::size_t size) {
    return memPool3.allocate();
}


void Object3::operator delete(void* mem) {
    memPool3.deallocate(mem);
}


void test3() {
    for (std::size_t i = 0; i < MAX_OBJECTS; ++i) {
        data3[i] = new Object3;
    }
    for (std::size_t i = 0; i < MAX_OBJECTS; ++i) {
        delete data3[i];
    }
}


int main() {
    double dur = timeFunction(test3);
    std::cout << std::fixed << dur << " seconds\n";
    system("pause");
    return 0;
}
