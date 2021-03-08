#ifndef GCLIB_GCMALLOC_HPP
#define GCLIB_GCMALLOC_HPP


#include <cstddef>
#include "gctraits.hpp"


/**
 * Malloc function for type T.
 * @param T type of object to allocate/deallocate memory for.
 */
template <class T> struct GCMalloc {
    /**
     * Allocates memory for type T.
     * It statically selects T::operator new if it exists, otherwise it uses ::operator new.
     * @param size number of objects to allocate.
     * @return pointer to allocated memory.
     */
    static void* malloc(size_t size) {
        if constexpr (GCHasOperatorNew<T>::Value) {
            static_assert(GCHasOperatorDelete<T>::Value, "class has operator new but not operator delete");
            return T::operator new(size);
        }
        else {
            return ::operator new(size);
        }
    }

    /**
     * Frees memory for type T. 
     * It statically selects T::operator delete if it exists, otherwise it uses ::operator delete.
     * @param mem pointer to memory start to free, as returned by malloc.
     */
    static void free(void* mem) {
        if constexpr (GCHasOperatorDelete<T>::Value) {
            T::operator delete(mem);
        }
        else {
            ::operator delete(mem);
        }
    }
};


/**
 * Malloc function for type T[].
 * @param T type of object to allocate/deallocate memory for.
 */
template <class T> struct GCMalloc<T[]> {
    /**
     * Allocates memory for type T.
     * It statically selects T::operator new[] if it exists, otherwise it uses ::operator new[].
     * @param size number of objects to allocate.
     * @return pointer to allocated memory.
     */
    static void* malloc(size_t size) {
        if constexpr (GCHasOperatorNew<T[]>::Value) {
            static_assert(GCHasOperatorDelete<T[]>::Value, "class has operator new[] but not operator delete[]");
            return T::operator new[](size);
        }
        else {
            return ::operator new[](size);
        }
    }

    /**
     * Frees memory for type T. 
     * It statically selects T::operator delete[] if it exists, otherwise it uses ::operator delete[].
     * @param mem pointer to memory start to free, as returned by malloc.
     */
    static void free(void* mem) {
        if constexpr (GCHasOperatorDelete<T[]>::Value) {
            T::operator delete[](mem);
        }
        else {
            ::operator delete[](mem);
        }
    }
};


#endif //GCLIB_GCMALLOC_HPP
