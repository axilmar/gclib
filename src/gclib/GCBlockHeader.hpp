#ifndef GCLIB_GCBLOCKHEADER_HPP
#define GCLIB_GCBLOCKHEADER_HPP


#include "gclib/GCPtrStruct.hpp"
#include "gclib/GCList.hpp"
#include "gclib/GCIBlockHeaderVTable.hpp"


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

    ///vtable that manages this block header
    GCIBlockHeaderVTable& vtable;
        
    ///thread data the block belongs to.
    struct GCThreadData* owner;

    ///constructor.
    GCBlockHeader(std::size_t size, GCIBlockHeaderVTable& vtable, struct GCThreadData* owner)
        : end(reinterpret_cast<char*>(this) + size)
        , vtable(vtable)
        , owner(owner)
    {
    }
};


#endif //GCLIB_GCBLOCKHEADER_HPP
