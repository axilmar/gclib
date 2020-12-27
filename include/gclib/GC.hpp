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

    /**
     * Returns the current allocation size.
     * @return the current allocation size.
     */
    static std::size_t getAllocSize();

    /**
     * Returns the current allocation limit.
     * @return the current allocation limit.
     */
    static std::size_t getAllocLimit();

    /**
     * Sets the current allocation limit.
     * @param limit new allocation limit.
     */
    static void setAllocLimit(std::size_t limit);
};


#endif //GCLIB_GC_HPP
