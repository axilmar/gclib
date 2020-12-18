#include "gclib/make.hpp"
#include "Collector.hpp"


namespace gclib {


    //allocates a memory block from the collector.
    void* MakePrivate::allocate(const std::size_t size, IObjectManager* om) {
        return Collector::allocate(size, om);
    }


    //marks a block as invalid
    void MakePrivate::markInvalid(void* mem) {
        Collector::markInvalid(mem);
    }


    //marks a block as valid
    void MakePrivate::markValid(void* mem) {
        Collector::markValid(mem);
    }


} //namespace gclib
