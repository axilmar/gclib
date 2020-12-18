#ifndef GCLIB_COLLECTOR_HPP
#define GCLIB_COLLECTOR_HPP


#include <mutex>
#include <atomic>
#include "gclib/DList.hpp"
#include "gclib/IObjectManager.hpp"
#include "ExecMutex.hpp"


namespace gclib {


    /**
        The collector class.
     */
    class Collector {
    public:
        /**
            Heap start.
         */
        char* heapStart{ nullptr };

        /**
            heap top.
         */
        std::atomic<char*> heapTop{ nullptr };

        /**
            Heap end.
         */
        char* heapEnd{ nullptr };

        /**
            Mutex for the thread list.
         */
        std::mutex mutex;

        /**
            list of known threads.
         */
        DList<class Thread> threads;

        /**
            List of thread data from terminated threads.
         */
        DList<class ThreadData> threadData;

        /**
            Collection execution mutex.
         */
        ExecMutex collectionMutex;

        /**
            all blocks are gathered here for quick finding of objects using binary search.
         */
        std::vector<class Block*> allBlocks;

        /**
            Deletes all objects and frees the heap memory.
         */
        ~Collector();

        /**
            Returns the one and only instance of the collector.
            @return the one and only instance of the collector.
         */
        static Collector& instance() noexcept;

        /**
            Initializes the collector.
            @param heapSize heap size, in bytes; by default, its 256 MB.
            @exception std::logic_error thrown if the gc is already initialized.
            @exception std::invalid_argument thrown if the heap size is too small (i.e. less than 256 bytes).
         */
        static void initialize(const std::size_t heapSize);

        /**
            Allocates a garbage-collected memory block.
            @param size number of bytes to allocate.
            @param om object manager.
            @return pointer to memory block.
            @exception std::bad_alloc thrown if there is not enough memory.
         */
        static void* allocate(std::size_t size, IObjectManager* om);

        /**
            Marks a block as invalid, when an exception happens in its constructor.
            @param mem pointer to memory as returned by allocate().
         */
        static void markInvalid(void* mem);

        /**
            Marks a block as valid.
            @param mem pointer to memory as returned by allocate().
         */
        static void markValid(void* mem);

        /**
            Collects garbage.
            @return true if memory was released, false otherwise.
         */
        static bool collectGarbage();
    };


} //namespace gclib


#endif //GCLIB_COLLECTOR_HPP
