#include "Collector.hpp"
#include "Thread.hpp"
#include "gclib/IObjectManager.hpp"
#include "gclib/OutOfMemoryException.hpp"


namespace gclib {


    //block status
    enum BlockStatus {
        //not yet known if construction has been completed or not
        Unknown = -2,

        //invalid; an exception in constructor prevented the collector from collecting the object
        Invalid = -1,

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
            : status{Unknown} , end((char*)this + size) , objectManager(om)
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

        //check against heap end; if valid, then return it
        if (heapPos + size <= collector.heapEnd) {
            return new (heapPos) Block(size, om) + 1;
        }

        return nullptr;
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

        //allocate memory
        collector.heapStart = (char*)::operator new(heapSize);
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
        //TODO
        return false;
    }


} //namespace gclib
