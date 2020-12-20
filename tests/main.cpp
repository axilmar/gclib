#include <iostream>
#include <chrono>


template <class F> double timeFunction(F&& func) {
    auto start = std::chrono::high_resolution_clock::now();
    func();
    auto end = std::chrono::high_resolution_clock::now();
    return std::chrono::duration_cast<std::chrono::duration<double>>(end - start).count();
}


#include "gclib/MemoryPool.hpp"


namespace gclib {


    template <std::size_t MinBlockSize = sizeof(void*)*2,
        std::size_t MaxBlockSize = 4096,
        std::size_t BlockIncrement = sizeof(void*) * 2,
        std::size_t ChunkSize = 32>
        class MemoryResource {
        public:
            static_assert(MinBlockSize > 0, "MinBlockSize should be greater than 0");
            static_assert(MinBlockSize >= BlockIncrement, "MinBlockSize should be greater than or equal to BlockIncrement");
            static_assert((MinBlockSize & (sizeof(void*) - 1)) == 0, "MinBlockSize should have pointer alignment");
            static_assert((MaxBlockSize & (sizeof(void*) - 1)) == 0, "MaxBlockSize should have pointer alignment");
            static_assert(MinBlockSize <= MaxBlockSize, "MinBlockSize should be less than or equal to MaxBlockSize");
            static_assert((BlockIncrement & (sizeof(void*) - 1)) == 0, "BlockIncrement should have pointer alignment");

            MemoryResource() {
                initialize();
            }

            void* allocate(const std::size_t size) {
                const std::size_t roundedSize = (size + BlockIncrement - 1) & ~(BlockIncrement - 1);
                if (roundedSize <= MaxBlockSize) {
                    Block* const block = (Block*)m_memoryPoolArray.allocate(roundedSize);
                    return initBlock(block, roundedSize);
                }
                Block* block = (Block*)::operator new(roundedSize);
                return initBlock(block, roundedSize);
            }

            void deallocate(void* const mem) {
                Block* const block = (Block*)mem - 1;
                if (block->size <= MaxBlockSize) {
                    m_memoryPoolArray.deallocate(block->size, block);
                    return;
                }
                ::operator delete(block);
            }

        private:
            struct Block {
                std::size_t size;
            };

            static constexpr std::size_t MemoryPoolCount = (MaxBlockSize - MinBlockSize) / BlockIncrement;

            char m_data[sizeof(MemoryPool<4>) * MemoryPoolCount];

            void initialize() {
                for (std::size_t index = 0; index < MemoryPoolCount; ++index) {
                    void* poolMem = m_data + index * sizeof(MemoryPool<4>);
                    new (poolMem) MemoryPool<4>;
                }
            }
    };


} //namespace gclib


using namespace gclib;


int main() {
    MemoryResource<> mr;
    void* mem = mr.allocate(1);
    mr.deallocate(mem);
    system("pause");
    return 0;
}
