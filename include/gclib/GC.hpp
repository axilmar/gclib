#ifndef GCLIB_GC_HPP
#define GCLIB_GC_HPP


/**
 * Interface to the collector.
 */
class GC {
public:
    /**
     * Collects garbage synchronously.
     * @return number of allocated bytes after the collection.
     */
    static std::size_t collect();

    /**
     * Collects data asynchronously. 
     */
    static void collectAsync();
};


#endif //GCLIB_GC_HPP
