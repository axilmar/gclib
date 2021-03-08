#ifndef GCLIB_GCSHAREDSCANNER_HPP
#define GCLIB_GCSHAREDSCANNER_HPP


#include "GCISharedScanner.hpp"



/**
 * Shared scanner implementation.
 * @param T type of object to scan.
 */
template <class T> class GCSharedScanner : public GCISharedScanner {
public:
    /**
     * Checks if the object or objects within the given memory region are still shared.
     * @param start memory start.
     * @param end memory end.
     * @return true if there is at least one object shared within the given memory region,
     * false otherwise.
     */
    bool isShared(void* start, void* end) const final {
        for (T* obj = static_cast<T*>(start); obj < end; ++obj) {
            if (isShared(obj)) {
                return true;
            }
        }
        return false;
    }

private:
    static bool isShared(T* object) {
        return !object->weak_from_this().expired();
    }
};


#endif //GCLIB_GCSHAREDSCANNER_HPP
