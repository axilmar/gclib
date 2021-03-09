#ifndef GCLIB_GCBLOCKHEADERVTABLE_HPP
#define GCLIB_GCBLOCKHEADERVTABLE_HPP


#include <memory>
#include <type_traits>
#include "GCIBlockHeaderVTable.hpp"
#include "GCIScannableObject.hpp"
#include "gcmalloc.hpp"


/**
 * Default block header vtable implementation for a single object of a given type.
 * @param T type of object to manage.
 */
template <class T> class GCBlockHeaderVTable : public GCIBlockHeaderVTable {
public:
    /**
     * Scans for member pointers.
     * If T implements the interface GCIScannableObject, then it calls 
     * the scan function of that interface, otherwise it does nothing.
     * @param start memory start.
     * @param end memory end.
     */
    void scan(void* start, void* end) noexcept final {
        if constexpr (std::is_base_of_v<GCIScannableObject, T>) {
            reinterpret_cast<T*>(start)->GCIScannableObject::scan();
        }
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

    /**
     * Checks if the object of the given type is shared by std::shared_ptr.
     * If the type inherits from std::enable_shared_from_this,
     * then if it's weak pointer is not expired, then it is considered shared,
     * otherwise it is not.
     * @param start start of memory block.
     * @param end end of memory block.
     * @return true if objects have shared pointers to them, false otherwise.
     */
    bool shared(void* start, void* end) const noexcept final {
        if constexpr (std::is_base_of_v<std::enable_shared_from_this<T>, T>) {
            return !reinterpret_cast<T*>(start)->std::enable_shared_from_this<T>::weak_from_this().expired();
        }
        else {
            return false;
        }
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
     * If T implements the interface GCIScannableObject, then it calls 
     * the scan function of that interface,for each object of the array,
     * otherwise it does nothing.
     * @param start memory start.
     * @param end memory end.
     */
    void scan(void* start, void* end) noexcept final {
        if constexpr (std::is_base_of_v<GCIScannableObject, T>) {
            for (T* obj = reinterpret_cast<T*>(start); obj < end; ++obj) {
                reinterpret_cast<T*>(obj)->GCIScannableObject::scan();
            }
        }
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

    /**
     * Checks if the objects of the given type is shared by std::shared_ptr.
     * If the type inherits from std::enable_shared_from_this,
     * then if one of the objects' weak pointer is not expired, then it is considered shared,
     * otherwise it is not.
     * @param start start of memory block.
     * @param end end of memory block.
     * @return true if objects have shared pointers to them, false otherwise.
     */
    bool shared(void* start, void* end) const noexcept final {
        if constexpr (std::is_base_of_v<std::enable_shared_from_this<T>, T>) {
            for (T* obj = reinterpret_cast<T*>(start); obj < end; ++obj) {
                if (!obj->std::enable_shared_from_this<T>::weak_from_this().expired()) {
                    return true;
                }
            }
            return false;
        }
        else {
            return false;
        }
    }
};


#endif //GCLIB_GCBLOCKHEADERVTABLE_HPP
