#ifndef GCLIB_GCNEW_HPP
#define GCLIB_GCNEW_HPP


#include <new>
#include "GCPtr.hpp"


///class with private algorithms used by the gcnew template function.
class GCNew {
private:
    //thread lock + ptr list store
    struct Lock {
        void* prevPtrList;
        Lock();
        ~Lock();
    };

    //finalizer
    template <class T> struct Finalizer {
        static void proc(void* start, void* end) {
            reinterpret_cast<T*>(start)->~T();
        }
    };

    //allocate gc memory
    static void* allocate(std::size_t size, void(*finalizer)(void*, void*), void*&prevPtrList);

    template <class T, class... Args> friend void* gcnew(Args&&...);
};


/**
 * Allocates a garbage-collected object.
 * @param args arguments to pass to the object's constructor.
 * @return pointer to the object.
 */
template <class T, class... Args> GCPtr<T> gcnew(Args&&... args) {
    GCNew::Lock lock;
    void* mem = GCNew::allocate(sizeof(T), &GCNew::Finalizer<T>::proc, lock.prevPtrList);
    new (mem) T(std::forward<Args>(args)...);
    return mem;
}


#endif //GCLIB_GCNEW_HPP
