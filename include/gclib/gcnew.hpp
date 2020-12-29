#ifndef GCLIB_GCNEW_HPP
#define GCLIB_GCNEW_HPP


#include <new>
#include <functional>
#include "GCPtr.hpp"


template <class T> struct GCList;


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

    //test if a class has operator new
    template <class T> struct HasOperatorNew {
    private:
        template <class C> static char test(decltype(&C::operator new));
        template <class C> static int test(...);

    public:
        static constexpr bool Value = sizeof(test<T>(0)) == sizeof(char);
    };

    //test if a class has operator delete
    template <class T> struct HasOperatorDelete {
    private:
        template <class C> static char test(decltype(&C::operator delete));
        template <class C> static int test(...);

    public:
        static constexpr bool Value = sizeof(test<T>(0)) == sizeof(char);
    };

    //test if a class has operator new[]
    template <class T> struct HasOperatorNewArray {
    private:
        template <class C> static char test(decltype(&C::operator new[]));
        template <class C> static int test(...);

    public:
        static constexpr bool Value = sizeof(test<T>(0)) == sizeof(char);
    };

    //test if a class has operator delete[]
    template <class T> struct HasOperatorDeleteArray {
    private:
        template <class C> static char test(decltype(&C::operator delete[]));
        template <class C> static int test(...);

    public:
        static constexpr bool Value = sizeof(test<T>(0)) == sizeof(char);
    };

    //if the allocation limit is exceeded, then collect garbage
    static void collectGarbageIfAllocationLimitIsExceeded();

    //returns the block header size
    static std::size_t getBlockHeaderSize();

    //register gc memory
    static void* registerAllocation(std::size_t size, void* mem, void(*finalizer)(void*, void*), std::function<void(void*)>&& free, GCList<GCPtrStruct>*& prevPtrList);

    //unregister gc memory
    static void unregisterAllocation(void* mem);

    //sets the current pointer list
    static void setPtrList(GCList<GCPtrStruct>* ptrList);

    //deallocate gc memory
    static void deallocate(void* mem);

    //allocates memory for the given type
    template <class T> static void* malloc(size_t size) {
        if constexpr (HasOperatorNew<T>::Value) {
            static_assert(HasOperatorDelete<T>::Value, "class has operator new but not operator delete");
            return T::operator new(size);
        }
        else {
            return ::operator new(size);
        }
    }

    //frees memory for the given type
    template <class T> static void free(void* mem) {
        if constexpr (HasOperatorDelete<T>::Value) {
            T::operator delete(mem);
        }
        else {
            ::operator delete(mem);
        }
    }

    //allocates array memory for the given type
    template <class T> static void* mallocArray(size_t size) {
        if constexpr (HasOperatorNewArray<T>::Value) {
            static_assert(HasOperatorDeleteArray<T>::Value, "class has operator new[] but not operator delete[]");
            return T::operator new[](size);
        }
        else {
            return ::operator new[](size);
        }
    }

    //frees array memory for the given type
    template <class T> static void freeArray(void* mem) {
        if constexpr (HasOperatorDeleteArray<T>::Value) {
            T::operator delete[](mem);
        }
        else {
            ::operator delete[](mem);
        }
    }

    //create objects
    template <class T, class Malloc, class Init, class Free> static GCPtr<T> create(std::size_t size, Malloc&& malloc, Init&& init, Free&& free) {
        //before any allocation, check if the allocation limit is exceeded;
        //if so, then collect garbage
        GCNewPriv::collectGarbageIfAllocationLimitIsExceeded();

        //prevent the collector from running until the result pointer is registered to the collector;
        //otherwise the new object might be collected prematurely
        GCNewPriv::Lock lock;

        //previous pointer list is stored here
        GCList<GCPtrStruct>* prevPtrList;

        //include the block header in the allocation
        size += getBlockHeaderSize();

        //allocate memory
        void* allocMem = malloc(size);

        //on allocation failure, throw exception
        if (!allocMem) {
            throw GCBadAlloc();
        }

        //register allocation
        void* objectMem = GCNewPriv::registerAllocation(size, allocMem, &GCNewPriv::Finalizer<T>::proc, std::forward<Free>(free), prevPtrList);

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
            free(allocMem);
            throw;
        }

        return nullptr;
    }


    template <class T, class... Args> friend GCPtr<T> gcnew(Args&&...);
};


/**
 * Allocates a garbage-collected object.
 * @param args arguments to pass to the object's constructor.
 * @return pointer to the object.
 * @exception GCBadAlloc thrown if memory allocation fails.
 */
template <class T, class... Args> GCPtr<T> gcnew(Args&&... args) {
    return GCNewPriv::create<T>(
        sizeof(T), 

        //malloc
        [](std::size_t size) {
            return GCNewPriv::malloc<T>(size);
        },
        
        //init
        [&](void* mem) { 
            return ::new(mem) T(std::forward<Args>(args)...); 
        },

        //free
        [](void* mem) {
            GCNewPriv::free<T>(mem);
        }
    );
}


#endif //GCLIB_GCNEW_HPP
