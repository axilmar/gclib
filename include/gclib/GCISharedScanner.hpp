#ifndef GCLIB_GCISHAREDSCANNER_HPP
#define GCLIB_GCISHAREDSCANNER_HPP


/**
 * Shared scanner interface.
 * It allows objects that inherit from std::enabled_shared_from_this 
 * to be kept from being collected if there are shared pointers and no gc pointers to it.
 */
class GCISharedScanner {
public:
    /**
     * Checks if the object or objects within the given memory region are still shared.
     * @param start memory start.
     * @param end memory end.
     * @return true if there is at least one object shared within the given memory region,
     * false otherwise.
     */
    virtual bool isShared(void* start, void* end) const = 0;
};


#endif //GCLIB_GCISHAREDSCANNER_HPP
