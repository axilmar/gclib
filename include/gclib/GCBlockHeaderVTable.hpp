#ifndef GCLIB_GCBLOCKHEADERVTABLE_HPP
#define GCLIB_GCBLOCKHEADERVTABLE_HPP


#include "GCIBlockHeaderVTable.hpp"
#include "gcmalloc.hpp"


/**
 * Default block header vtable implementation for a single object of a given type.
 * @param T type of object to manage.
 */
template <class T> class GCBlockHeaderVTable : public GCIBlockHeaderVTable {
public:
    /**
     * Scans for member pointers.
     * Empty by default; the collector will scan for gc pointers by default.
     * @param start memory start.
     * @param end memory end.
     */
    void scan(void* start, void* end) noexcept final {
    }

    /**
     * Finalizes the object by calling its destructor.
     * @param start memory start.
     * @param end memory end.
     */
    void finalize(void* start, void* end) noexcept final {
        reinterpret_cast<T*>(start)->~T();
    }

    /**
     * Frees memory either by using T::operator delete or global operator delete.
     * @param mem pointer to memory to free.
     */
    void free(void* mem) noexcept final {
        GCMalloc<T>::free(mem);
    }
};


/**
 * Default block header vtable implementation for an array of objects of the given type.
 * @param T type of object to manage.
 */
template <class T> class GCBlockHeaderVTable<T[]> : public GCIBlockHeaderVTable {
public:
    /**
     * Scans for member pointers.
     * Empty by default; the collector will scan for gc pointers by default.
     * @param start memory start.
     * @param end memory end.
     */
    void scan(void* start, void* end) noexcept final {
    }

    /**
     * Finalizes the objects by calling their destructors in reverse order.
     * @param start memory start.
     * @param end memory end.
     */
    void finalize(void* start, void* end) noexcept final {
        for (T* obj = reinterpret_cast<T*>(end) - 1; obj >= start; --obj) {
            reinterpret_cast<T*>(obj)->~T();
        }
    }

    /**
     * Frees memory either by using T::operator delete[] or global operator delete[].
     * @param mem pointer to memory to free.
     */
    void free(void* mem) noexcept final {
        GCMalloc<T[]>::free(mem);
    }
};


#endif //GCLIB_GCBLOCKHEADERVTABLE_HPP
