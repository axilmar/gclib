#ifndef GCLIB_GCNEW_HPP
#define GCLIB_GCNEW_HPP


#include <new>
#include "GCThreadLock.hpp"
#include "GCBlockHeaderVTable.hpp"
#include "GCPtr.hpp"


template <class T> struct GCList;


///class with private algorithms used by the gcnew template function.
class GCNewPriv {
private:
    //if the allocation limit is exceeded, then collect garbage
    static void collectGarbageIfAllocationLimitIsExceeded();

    //returns the block header size
    static std::size_t getBlockHeaderSize();

    //register gc memory; returns pointer to object memory
    static void* registerAllocation(std::size_t size, void* mem, GCIBlockHeaderVTable& vtable, GCList<GCPtrStruct>*& prevPtrList);

    //finalizes the given block
    static void finalize(void* mem);

    //unregister gc memory
    static void unregisterAllocation(void* mem);

    //sets the current pointer list
    static void setPtrList(GCList<GCPtrStruct>* ptrList);

    //unregister and deallocate gc memory
    static void deallocate(void* mem);

    template <class T, class Malloc, class Init, class VTable> friend GCPtr<T> gcnew(std::size_t, Malloc&&, Init&&, VTable&);
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


/**
 * The main function used for allocating garbage collected memory.
 * If the allocation limit is exceeded, it starts a parallel collection.
 * @param size number of bytes to allocate.
 * @param malloc function to use for allocating memory.
 * @param init function to use for initializing objects.
 * @param vtable reference to vtable that is used to scan/finalize/free memory.
 * @return garbage-collected pointer to object.
 * @exception GCBadAlloc thrown if malloc returns null.
 * @exception other thrown from object construction.
 */
template <class T, class Malloc, class Init, class VTable> GCPtr<T> gcnew(std::size_t size, Malloc&& malloc, Init&& init, VTable& vtable) {

    //before any allocation, check if the allocation limit is exceeded; if so, then collect garbage
    GCNewPriv::collectGarbageIfAllocationLimitIsExceeded();

    //prevent the collector from running until the result pointer is registered to the collector;
    //otherwise the new object might be collected prematurely
    GCThreadLock lock;

    //previous pointer list is stored here
    GCList<GCPtrStruct>* prevPtrList;

    //include the block header in the allocation
    size += GCNewPriv::getBlockHeaderSize();

    //allocate memory
    void* allocMem = malloc(size);

    //on allocation failure, throw exception
    if (!allocMem) {
        throw GCBadAlloc();
    }

    //register allocation
    void* objectMem = GCNewPriv::registerAllocation(size, allocMem, vtable, prevPtrList);

    //initialize the objects
    try {
        T* result = init(objectMem);
        GCNewPriv::setPtrList(prevPtrList);
        return result;
    }

    //catch any exceptions during construction in order to undo the allocation changes
    catch (...) {
        GCNewPriv::setPtrList(prevPtrList);
        GCNewPriv::unregisterAllocation(allocMem);
        vtable.free(allocMem);
        throw;
    }

    return nullptr;
}


/**
 * Allocates a garbage-collected object.
 * @param args arguments to pass to the object's constructor.
 * @return pointer to the object.
 * @exception GCBadAlloc thrown if memory allocation fails.
 */
template <class T, class... Args> GCPtr<T> gcnew(Args&&... args) {
    static GCBlockHeaderVTable<T> vtable;

    return gcnew<T>(
        sizeof(T), 

        //malloc
        [](std::size_t size) {
            return GCMalloc<T>::malloc(size);
        },
        
        //init
        [&](void* mem) { 
            return ::new(mem) T(std::forward<Args>(args)...); 
        },

        //vtable
        vtable
    );
}


/**
 * Allocates a garbage-collected array of objects.
 * @param count number of elements of the array.
 * @param args arguments to pass to each object's constructor.
 * @return pointer to the object.
 * @exception GCBadAlloc thrown if memory allocation fails.
 */
template <class T, class... Args> GCPtr<T> gcnewArray(std::size_t count, Args&&... args) {
    static GCBlockHeaderVTable<T[]> vtable;

    return gcnew<T>(
        count * sizeof(T), 

        //malloc
        [](std::size_t size) {
            return GCMalloc<T[]>::malloc(size);
        },
        
        //init
        [&](void* mem) {
            for (T *obj = reinterpret_cast<T*>(mem), *end = obj + count; obj < end; ++obj) {
                ::new(obj) T(std::forward<Args>(args)...);
            }
            return reinterpret_cast<T*>(mem);
        },

        //vtable
        vtable
    );
}


/**
 * Invokes the given object's destructor or the destructors of each object in the array
 * and deallocates the memory block pointed to by the given pointer.
 * @param ptr ptr to object to deallocate; it shall point to a garbage-collected object or a garbage-collected array of objects.
 */
template <class T> void gcdelete(const GCPtr<T>& ptr) {
    //if the pointer is null, do nothing else
    if (!ptr) {
        return;
    }

    //finalize the object or objects
    GCNewPriv::finalize(ptr.get());

    //remove the block from the collector
    GCThreadLock lock;
    GCNewPriv::deallocate(ptr);
}


/**
 * Same as gcdelete(ptr). 
 */
template <class T> void gcdeleteArray(const GCPtr<T>& ptr) {
    gcdelete(ptr);
}


#endif //GCLIB_GCNEW_HPP
