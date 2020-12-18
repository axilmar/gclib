#ifndef GCLIB_IOBJECTMANAGER_HPP
#define GCLIB_IOBJECTMANAGER_HPP


#include "IPtrManager.hpp"


namespace gclib {


    /**
        GC object manager interface.
     */
    class IObjectManager {
    public:
        /**
            Interface for memory scanning of an object.
            @param begin start of memory.
            @param end end of memory.
            @param pm pointer manager.
         */
        virtual void scanPtrs(void* begin, void* end, IPtrManager& pm) noexcept = 0;

        /**
            Interface for finalization.
            @param begin start of memory.
            @param end end of memory.
         */
        virtual void finalize(void* begin, void* end) noexcept = 0;
    };


} //namespace gclib


#endif //GCLIB_IOBJECTMANAGER_HPP
