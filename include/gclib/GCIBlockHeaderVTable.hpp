#ifndef GCLIB_GCIBLOCKHEADERVTABLE_HPP
#define GCLIB_GCIBLOCKHEADERVTABLE_HPP


/**
 * VTable interface for block headers. 
 */
class GCIBlockHeaderVTable {
public:
    /**
     * Scan for pointers interface.
     * @param start memory start.
     * @param end memory end.
     */
    virtual void scan(void* start, void* end) noexcept = 0;

    /**
     * Finalize interface.
     * @param start memory start.
     * @param end memory end.
     */
    virtual void finalize(void* start, void* end) noexcept = 0;

    /**
     * Free memory interface.
     * @param mem pointer to memory to free.
     */
    virtual void free(void* mem) noexcept = 0;

    /**
     * Interface for checking if the object is or objects are shared via shared pointers.
     * Objects that are shared by shared pointers at the time of collection are not collected.
     * @param start start of memory block.
     * @param end end of memory block.
     * @return true if objects have shared pointers to them, false otherwise.
     */
    virtual bool shared(void* start, void* end) const noexcept = 0;
};


#endif //GCLIB_GCIBLOCKHEADERVTABLE_HPP
