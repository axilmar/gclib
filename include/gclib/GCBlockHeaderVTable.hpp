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


/**
 * Class that allows the customization of the individual functions of a block header vtable.
 * @param Scan type of the scan function.
 * @param Finalize type of the finalize function.
 * @param Free type of the free function.
 */
template <class Scan, class Finalize, class Free> class GCCustomBlockHeaderVTable : public GCIBlockHeaderVTable {
public:
    /**
     * Constructor.
     * @param scan function.
     * @param finalize finalize function.
     * @param free free function.
     */
    GCCustomBlockHeaderVTable(Scan&& scan, Finalize&& finalize, Free&& free)
        : m_scan(std::move(scan))
        , m_finalize(std::move(finalize))
        , m_free(std::move(free))
    {
    }

    /**
     * Scans for member pointers by invoking the scan function.
     * @param start memory start.
     * @param end memory end.
     */
    void scan(void* start, void* end) noexcept final {
        m_scan(start);
    }

    /**
     * Finalizes the objects by invoking the finalize function.
     * @param start memory start.
     * @param end memory end.
     */
    void finalize(void* start, void* end) noexcept final {
        m_finalize(start);
    }

    /**
     * Frees memory either by invoking the free function.
     * @param mem pointer to memory to free.
     */
    void free(void* mem) noexcept final {
        m_free(mem);
    }

private:
    Scan m_scan;
    Finalize m_finalize;
    Free m_free;
};


#endif //GCLIB_GCBLOCKHEADERVTABLE_HPP
