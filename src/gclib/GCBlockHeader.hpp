#ifndef GCLIB_GCBLOCKHEADER_HPP
#define GCLIB_GCBLOCKHEADER_HPP


#include "gclib/GCPtrStruct.hpp"
#include "gclib/GCList.hpp"


/**
 * Data that preceed a heap-allocated object.
 */
class GCBlockHeader : public GCNode<GCBlockHeader> {
public:
    ///member ptrs of this block.
    GCList<GCPtrStruct> ptrs;

    ///end of block.
    void* end;

    ///finalizer.
    void(*finalizer)(void*, void*);

    ///mutex of the owner thread.
    std::recursive_mutex* mutex;
};


#endif //GCLIB_GCBLOCKHEADER_HPP
