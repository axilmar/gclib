#include <stdexcept>
#include "gclib/GC.hpp"
#include "Collector.hpp"


namespace gclib {


    //Initializes the collector.
    void GC::initialize(const std::size_t heapSize) {
        Collector& collector = Collector::instance();

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
        collector.heapEnd = collector.heapStart + heapSize;
    }


} //namespace gclib
