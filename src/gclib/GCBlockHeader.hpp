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
    size_t cycle{ 0 };

    ///vtable that manages this block header
    GCIBlockHeaderVTable& vtable;
        
    ///thread data the block belongs to.
    struct GCThreadData* owner;

    //shared scanner; set only if the block can contain objects that have shared ptrs to them
    class GCISharedScanner* sharedScanner;

    ///constructor.
    GCBlockHeader(size_t size, GCIBlockHeaderVTable& vtable, struct GCThreadData* owner)
        : end(reinterpret_cast<char*>(this) + size)
        , vtable(vtable)
        , owner(owner)
        , sharedScanner(nullptr)
    {
    }
};


#endif //GCLIB_GCBLOCKHEADER_HPP
