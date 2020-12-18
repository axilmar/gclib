#include "Collector.hpp"


namespace gclib {


    //the one and only instance of the collector.
    static Collector collector;


    //Returns the one and only instance of the collector.
    Collector& Collector::instance() noexcept {
        return collector;
    }


    //Deletes all objects and frees the heap memory.
    Collector::~Collector() {
        //TODO
    }


} //namespace gclib
