#ifndef GCLIB_GCPTRSTRUCT_HPP
#define GCLIB_GCPTRSTRUCT_HPP


#include <mutex>
#include "GCNode.hpp"


/**
 * Internal struct that represents a pointer.
 */
struct GCPtrStruct : GCNode<GCPtrStruct> {
    ///the pointer value.
    void* value;

    ///the mutex this pointer shall lock in order to copy/move values, register/unregister itself to/from the collector.
    std::recursive_mutex* mutex;
};


#endif //GCLIB_GCPTRSTRUCT_HPP
