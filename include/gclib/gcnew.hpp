#ifndef GCLIB_GCNEW_HPP
#define GCLIB_GCNEW_HPP


#include <new>
#include <type_traits>
#include "GCThreadLock.hpp"
#include "GCBlockHeaderVTable.hpp"
#include "GCPtr.hpp"
#include "GCSharedScanner.hpp"


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

    //register gc memory for shared object; returns pointer to object memory
    static void* registerAllocationShared(size_t size, void* mem, GCIBlockHeaderVTable& vtable, GCList<GCPtrStruct>*& prevPtrList, GCISharedScanner& sharedScanner);

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
template <class T, class Malloc, class Init, class VTable> GCPtr<T> gcnew(size_t size, Malloc&& malloc, Init&& init, VTable& vtable) {

    //before any allocation, check if the allocation limit is exceeded; if so, then collect garbage
    GCNewOperations::collectGarbageIfAllocationLimitIsExceeded();

    //prevent the collector from running until the result pointer is registered to the collector;
    //otherwise the new object might be collected prematurely
    GCThreadLock lock;

    //previous pointer list is stored here
    GCList<GCPtrStruct>* prevPtrList;

    //include the block header in the allocation
    size += GCNewOperations::getBlockHeaderSize();

    //allocate memory
    void* allocMem = malloc(size);

    //on allocation failure, throw exception
    if (!allocMem) {
        throw GCBadAlloc();
    }

    //register allocation
    void* objectMem;
    if constexpr (std::is_base_of_v<std::enable_shared_from_this<T>, T>) {
        static GCSharedScanner<T> sharedScanner;
        objectMem = GCNewOperations::registerAllocationShared(size, allocMem, vtable, prevPtrList, sharedScanner);
    }
    else {
        objectMem = GCNewOperations::registerAllocation(size, allocMem, vtable, prevPtrList);
    }

    //initialize the objects
    try {
        T* result = init(objectMem);
        GCNewOperations::setPtrList(prevPtrList);
        return result;
    }

    //catch any exceptions during construction in order to undo the allocation changes
    catch (...) {
        GCNewOperations::setPtrList(prevPtrList);
        GCDeleteOperations::unregisterAllocation(allocMem);
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
        [](size_t size) {
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
template <class T, class... Args> GCPtr<T> gcnewArray(size_t count, Args&&... args) {
    static GCBlockHeaderVTable<T[]> vtable;

    return gcnew<T>(
        count * sizeof(T), 

        //malloc
        [](size_t size) {
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
template <class T> void gcdelete(GCPtr<T>&& ptr) {
    GCDeleteOperations::gcdelete(ptr.reset());
}


/**
 * Same as gcdelete(ptr). 
 */
template <class T> void gcdeleteArray(GCPtr<T>&& ptr) {
    gcdelete(std::move(ptr));
}


#endif //GCLIB_GCNEW_HPP
