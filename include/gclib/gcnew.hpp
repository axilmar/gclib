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

    //finalizer
    template <class T> struct Finalizer {
        static void proc(void* start, void* end) {
            reinterpret_cast<T*>(start)->~T();
        }
    };

    //if the allocation limit is exceeded, then collect garbage
    static void collectGarbageIfAllocationLimitIsExceeded();

    //allocate gc memory
    static void* allocate(std::size_t size, void(*finalizer)(void*, void*), GCList<GCPtrStruct>*& prevPtrList);

    //sets the current pointer list
    static void setPtrList(GCList<GCPtrStruct>* ptrList);

    //deallocate gc memory
    static void deallocate(void* mem);

    //create objects
    template <class T, class Init> static GCPtr<T> create(const std::size_t size, Init&& init) {
        //before any allocation, check if the allocation limit is exceeded;
        //if so, then collect garbage
        GCNewPriv::collectGarbageIfAllocationLimitIsExceeded();

        //prevent the collector from running until the result pointer is registered to the collector;
        //otherwise the new object might be collected prematurely
        GCNewPriv::Lock lock;

        //previous pointer list is stored here
        GCList<GCPtrStruct>* prevPtrList;

        //allocate memory and set the current pointer list to point to the new block
        void* mem = GCNewPriv::allocate(size, &GCNewPriv::Finalizer<T>::proc, prevPtrList);

        try {
            //construct the object
            T* result = init(mem);

            //restore the pointer list
            GCNewPriv::setPtrList(prevPtrList);

            return result;
        }

        //catch any exceptions during construction in order to undo the allocation changes
        catch (...) {
            GCNewPriv::setPtrList(prevPtrList);
            GCNewPriv::deallocate(mem);
            throw;
        }

        return nullptr;
    }


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
    return GCNewPriv::create<T>(sizeof(T), [&](void* mem) { 
        return ::new(mem) T(std::forward<Args>(args)...); 
    });
}


#endif //GCLIB_GCNEW_HPP
