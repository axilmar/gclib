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

    //deallocate gc memory
    static void deallocate(void* mem);

    template <class T, class... Args> friend GCPtr<T> gcnew(Args&&...);
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
 * Allocates a garbage-collected object.
 * @param args arguments to pass to the object's constructor.
 * @return pointer to the object.
 * @exception GCBadAlloc thrown if memory allocation fails.
 */
template <class T, class... Args> GCPtr<T> gcnew(Args&&... args) {

    //before any allocation, check if the allocation limit is exceeded;
    //if so, then collect garbage
    GCNewPriv::collectGarbageIfAllocationLimitIsExceeded();

    //prevent the collector from running until the result pointer is registered to the collector;
    //otherwise the new object might be collected prematurely
    GCNewPriv::Lock lock;

    T* result;
    {
        //override the pointer list during construction (but not pointer return;
        //we want the return pointer to not be registered to the block's ptr list)
        GCNewPriv::PtrListOverride plo;

        //allocate memory
        void* mem = GCNewPriv::allocate(sizeof(T), &GCNewPriv::Finalizer<T>::proc, plo);

        //construct the object
        try {
            result = ::new (mem) T(std::forward<Args>(args)...);
        }
        catch (...) {
            GCNewPriv::deallocate(mem);
            throw;
        }
    }
    
    //return the allocated object
    return result;
}


#endif //GCLIB_GCNEW_HPP
