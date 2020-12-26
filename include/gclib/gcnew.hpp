#ifndef GCLIB_GCNEW_HPP
#define GCLIB_GCNEW_HPP


#include <new>
#include "GCPtr.hpp"


template <class T> struct GCList;


///class with private algorithms used by the gcnew template function.
class GCNewPriv {
private:
    //thread lock
    struct Lock {
        Lock();
        ~Lock();
    };

    //ptr list override
    struct PtrListOverride {
        GCList<GCPtrStruct>* prevPtrList;
        ~PtrListOverride();
    };

    //finalizer
    template <class T> struct Finalizer {
        static void proc(void* start, void* end) {
            reinterpret_cast<T*>(start)->~T();
        }
    };

    //if the allocation limit is exceeded, then collect garbage
    static void collectGarbageIfAllocationLimitIsExceeded();

    //allocate gc memory
    static void* allocate(std::size_t size, void(*finalizer)(void*, void*), PtrListOverride& plo);

    template <class T, class... Args> friend GCPtr<T> gcnew(Args&&...);
};


/**
 * Allocates a garbage-collected object.
 * @param args arguments to pass to the object's constructor.
 * @return pointer to the object.
 */
template <class T, class... Args> GCPtr<T> gcnew(Args&&... args) {

    //before any allocation, check if the allocation limit is exceeded;
    //if so, then collect garbage
    GCNewPriv::collectGarbageIfAllocationLimitIsExceeded();

    //prevent the collector from running until the result pointer is registered to the collector;
    //otherwise the new object might be collected prematurely
    GCNewPriv::Lock lock;

    T* obj;
    {
        //while a new block is allocated and constructed, override the thread's ptr list
        //to point to the list of the allocated block, so as that member pointers are 
        //added to the current block; use an object to do that, so as that the ptr list
        //override works also in case of exception
        GCNewPriv::PtrListOverride plo;

        //allocate block
        void* mem = GCNewPriv::allocate(sizeof(T), &GCNewPriv::Finalizer<T>::proc, plo);

        //initialize block
        obj = new (mem) T(std::forward<Args>(args)...);
    }
    
    return obj;
}


#endif //GCLIB_GCNEW_HPP
