#ifndef GCLIB_THREADDATA_HPP
#define GCLIB_THREADDATA_HPP


#include "gclib/Mutex.hpp"
#include "gclib/MemoryResource.hpp"
#include "gclib/VoidPtr.hpp"
#include "Block.hpp"


namespace gclib {


    //thread data; might live beyond the thread
    struct ThreadData : DNode<ThreadData> {
        //memory mutex
        Mutex memoryMutex;

        //data are allocated from this resource
        MemoryResource memoryResource{sizeof(Block) + sizeof(void*)};

        //pointer mutex
        Mutex ptrMutex;

        //pointers
        DList<VoidPtr> ptrs;

        //check if empty
        bool empty() const noexcept;

        //returns the current thread data instance
        static ThreadData& instance();
    };


    //each thread has this structure;
    //upon construction, it registers itself to the collector,
    //upon destruction, it unregisters itself from the collector.
    //It allows the collector to know which threads participate in collection.
    struct Thread {
        //thread data
        ThreadData* data = new ThreadData;

        //adds the data to the collector.
        Thread();

        //removes the data from the collector or moves them to the terminated data if not empty.
        ~Thread();
    };


} //namespace gclib


#endif //GCLIB_THREADDATA_HPP
