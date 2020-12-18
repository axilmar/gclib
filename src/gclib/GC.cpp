#include "gclib/GC.hpp"
#include "Collector.hpp"


namespace gclib {


    //Initializes the collector.
    void GC::initialize(const std::size_t heapSize) {
        Collector::initialize(heapSize);
    }


    //Collects garbage.
    void GC::collectGarbage() {
        Collector::collectGarbage();
    }


} //namespace gclib
