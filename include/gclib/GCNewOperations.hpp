#ifndef GCLIB_GCNEWOPERATIONS_HPP
#define GCLIB_GCNEWOPERATIONS_HPP


#include "GCBlockHeaderVTable.hpp"
#include "GCPtr.hpp"


template <class T> struct GCList;


///class with private algorithms used by the gcnew template function.
class GCNewOperations {
private:
    //if the allocation limit is exceeded, then collect garbage
    static void collectGarbageIfAllocationLimitIsExceeded();

    //returns the block header size
    static size_t getBlockHeaderSize();

    //register gc memory; returns pointer to object memory
    static void* registerAllocation(size_t size, void* mem, GCIBlockHeaderVTable& vtable, GCList<GCPtrStruct>*& prevPtrList);

    //sets the current pointer list
    static void setPtrList(GCList<GCPtrStruct>* ptrList);

    template <class T, class Malloc, class Init, class VTable> friend GCPtr<T> gcnew(size_t, Malloc&&, Init&&, VTable&);
    template <class T> friend void gcdelete(const GCPtr<T>&);
};


/**
 * Thrown if memory allocation for the collector failed.
 */
class GCBadAlloc : public std::bad_alloc {
public:
    /**
     * Returns the message for this exception.
     */
    const char* what() const noexcept final {
        return "GC memory allocation failed";
    }
};


#endif //GCLIB_GCNEWOPERATIONS_HPP
