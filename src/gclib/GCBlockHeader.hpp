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

    ///last gc cycle.
    std::size_t cycle{ 0 };

    ///finalizer.
    void(*finalizer)(void*, void*);

    ///thread data the block belongs to
    struct GCThreadData* owner;
};


#endif //GCLIB_GCBLOCKHEADER_HPP
