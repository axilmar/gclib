#ifndef GCLIB_COLLECTOR_HPP
#define GCLIB_COLLECTOR_HPP


namespace gclib {


    /**
        The collector class.
     */
    class Collector {
    public:
        /**
            Heap start.
         */
        char* heapStart{ nullptr };

        /**
            Heap end.
         */
        char* heapEnd{ nullptr };

        /**
            Returns the one and only instance of the collector.
            @return the one and only instance of the collector.
         */
        static Collector& instance() noexcept;

        /**
            Deletes all objects and frees the heap memory.
         */
        ~Collector();
    };


} //namespace gclib


#endif //GCLIB_COLLECTOR_HPP
