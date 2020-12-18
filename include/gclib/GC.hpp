#ifndef GCLIB_GC_HPP
#define GCLIB_GC_HPP


#include <cstddef>


namespace gclib {


    /**
        Class that provides static GC functions.
     */
    class GC {
    public:
        /**
            Initializes the collector.
            @param heapSize heap size, in bytes.
            @exception std::logic_error thrown if the gc is already initialized.
            @exception std::invalid_argument thrown if the heap size is too small (i.e. less than 256 bytes).
         */
        static void initialize(const std::size_t heapSize = 268'435'456);
    };


} //namespace gclib


#endif //GCLIB_GC_HPP
