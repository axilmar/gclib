#ifndef GCLIB_GC_HPP
#define GCLIB_GC_HPP


/**
 * Interface to the collector.
 */
class GC {
public:
    /**
     * Collects garbage. 
     */
    void collect();
};


#endif //GCLIB_GC_HPP
