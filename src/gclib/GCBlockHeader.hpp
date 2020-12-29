#ifndef GCLIB_GCBLOCKHEADER_HPP
#define GCLIB_GCBLOCKHEADER_HPP


#include <functional>
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

    ///frees memory
    std::function<void(void*)> free;

    ///thread data the block belongs to
    struct GCThreadData* owner;

    ///constructor
    GCBlockHeader(std::size_t size, void (*fin)(void*, void*), std::function<void(void*)>&& fr, struct GCThreadData* own)
        : end(reinterpret_cast<char*>(this) + size)
        , finalizer(fin)
        , free(std::move(fr))
        , owner(own)
    {
    }
};


#endif //GCLIB_GCBLOCKHEADER_HPP
