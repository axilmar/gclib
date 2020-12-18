#include "Collector.hpp"
#include "Thread.hpp"
#include "gclib/IObjectManager.hpp"
#include "gclib/OutOfMemoryException.hpp"


namespace gclib {


    //block status
    enum BlockStatus {
        //not yet known if construction has been completed or not
        Unknown,

        //invalid; an exception in constructor prevented the collector from collecting the object
        Invalid,

        //valid; this block was successfully constructed
        Valid
    };


    //block
    class Block {
    public:
        //status/new place
        union {
            std::atomic<BlockStatus> status;
            Block* newPlace;
        };

        //end of a block is also start of the next block
        union {
            char* end;
            Block* next;
        };

        //object manager for this block
        IObjectManager* objectManager;

        //constructor
        Block(std::size_t size, IObjectManager *om)
            : status{ Unknown }, end((char*)this + size), objectManager(om)
        {
        }
    };


    //the one and only instance of the collector.
    static Collector collector;


    //allocate some memory
    static void* allocateBlock(const std::size_t size, IObjectManager* om) {
        //lock the thread; this is due to having to check that
        //the returned address from 'heapTop' plus the given size does not go past 'heapEnd';
        //we don't want to do fetch_add on 'heapTop' while the collector is trying to gather
        //the table of blocks
        std::lock_guard lock(Thread::instance().mallocMutex);

        //get the next available heap position; threads enter this simultaneously
        char* heapPos = collector.heapTop.fetch_add(size, std::memory_order_acquire);

        //if the whole block fits into the heap, then return it
        if (heapPos + size <= collector.heapEnd) {
            return new (heapPos) Block(size, om) + 1;
        }

        //make the last block that falls out of the heap bounds invalid,
        //so as that it is not gathered
        reinterpret_cast<Block*>(heapPos)->status.store(Invalid, std::memory_order_release);

        return nullptr;
    }


    //stops threads
    static void stopThreads() {
        //disallow new threads
        collector.mutex.lock();

        //lock threads from allocating memory and from altering pointers
        for (Thread* thread = collector.threads.first(); thread != collector.threads.end(); thread = thread->next()) {
            thread->mallocMutex.lockForCollection();
            thread->mutex.lockForCollection();
        }

        //lock the thread data from altering pointers
        for (ThreadData* data = collector.threadData.first(); data != collector.threadData.end(); data = data->next()) {
            data->mutex.lockForCollection();
        }
    }


    //resumes threads
    static void resumeThreads() {
        //unlock the thread data
        for (ThreadData* data = collector.threadData.last(); data != collector.threadData.end(); data = data->prev()) {
            data->mutex.unlockForCollection();
        }

        //unlock the threads
        for (Thread* thread = collector.threads.last(); thread != collector.threads.end(); thread = thread->prev()) {
            thread->mutex.unlockForCollection();
            thread->mallocMutex.unlockForCollection();
        }

        //allow new threads
        collector.mutex.unlock();
    }


    //gathers all blocks
    static void gatherAllBlocks() {
        //iterate blocks until heap top
        for (Block* block = (Block*)collector.heapStart;;) {
            //stop if current block address reached the heap top struct
            if (block >= (Block*)collector.heapTop.load(std::memory_order_acquire)) {
                break;
            }

            //wait until the block reaches valid or invalid status,
            //i.e. wait for threads that are in initialization
            BlockStatus status;
            while ((status = block->status.load(std::memory_order_acquire)) == Unknown) {
                std::this_thread::yield();
            }

            //if the block status is valid, then add it to all blocks
            if (status == Valid) {
                collector.allBlocks.push_back(block);
            }

            //goto to next block
            block = block->next;
        }
    }


    //Deletes all objects and frees the heap memory.
    Collector::~Collector() {
        //TODO finalize all objects
        ::operator delete(heapStart);
    }


    //Returns the one and only instance of the collector.
    Collector& Collector::instance() noexcept {
        return collector;
    }


    //Initializes the collector.
    void Collector::initialize(const std::size_t heapSize) {
        //if the heap has already been set, throw exception
        if (collector.heapStart) {
            throw std::runtime_error("GC heap already created");
        }

        //if the heap size is too small, throw exception
        if (heapSize < 256) {
            throw std::invalid_argument("GC heap size too small");
        }


        //allocate memory;
        //add capacity for at least one pointer at the end,
        //in order to allow for making the last block 
        //that is out of bounds invalid
        collector.heapStart = (char*)::operator new(heapSize + sizeof(void*));
        collector.heapTop.store(collector.heapStart, std::memory_order_release);
        collector.heapEnd = collector.heapStart + heapSize;
    }


    //Allocates a garbage-collected memory block.
    void* Collector::allocate(std::size_t size, IObjectManager* om) {
        //add size of block; round size to ptr alignment
        size = (sizeof(Block) + size + sizeof(void*) - 1) & ~(sizeof(void*) - 1);

        for (;;) {
            //try to allocate memory
            void* mem = allocateBlock(size, om);

            //if memory was allocated, return it
            if (mem) {
                return mem;
            }

            //no memory was allocated; collect garbage
            if (collectGarbage()) {
                continue;
            }

            //no memory was freed; throw exception
            throw OutOfMemoryException();
        }
    }


    //Marks a block as invalid, when an exception happens in its constructor.
    void Collector::markInvalid(void* mem) {
        static_cast<Block*>(mem)[-1].status.store(Invalid, std::memory_order_release);
    }


    //Marks a block as valid.
    void Collector::markValid(void* mem) {
        static_cast<Block*>(mem)[-1].status.store(Valid, std::memory_order_release);
    }


    //Collects garbage.
    bool Collector::collectGarbage() {
        //avoid entering the collection thread simultaneously
        if (!collector.collectionMutex.lock()) {
            return true;
        }

        //stop all threads
        stopThreads();

        //gather all objects
        gatherAllBlocks();

        //mark objects as reachable

        bool freedSomeMemory = false;

        //resume threads
        resumeThreads();

        //allow further collections
        collector.collectionMutex.unlock();

        return freedSomeMemory;
    }


} //namespace gclib
