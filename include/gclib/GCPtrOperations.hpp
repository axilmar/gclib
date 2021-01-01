#ifndef GCLIB_GCPTROPERATIONS_HPP
#define GCLIB_GCPTROPERATIONS_HPP


#include "GCThreadLock.hpp"


/**
 * Class that provides a low-level interface for managing pointers. 
 */
class GCPtrOperations {
public:
    /**
     * Function that allows copying a pointer synchronized with the collector.
     * @param dst destination pointer.
     * @param src source pointer.
     */
    template <class Dst, class Src> static void copy(Dst*& dst, Src* src) {
        GCThreadLock lock;
        dst = src;
    }

    /**
     * Function that allows copying a pointer synchronized with the collector.
     * @param dst destination pointer.
     * @param src source pointer; set to null on return.
     */
    template <class Dst, class Src> static void move(Dst*& dst, Src*& src) {
        GCThreadLock lock;
        Src* temp = src;
        src = nullptr;
        dst = temp;
    }
};


#endif //GCLIB_GCPTROPERATIONS_HPP
 