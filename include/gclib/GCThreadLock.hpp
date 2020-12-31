#ifndef GCLIB_GCTHREADLOCK_HPP
#define GCLIB_GCTHREADLOCK_HPP


/**
 * Class that stops the collector from processing the data of the given thread.
 * It will block collection until it goes out of scope.
 */
class GCThreadLock {
public:
    /**
     * Blocks the collector from running.
     */
    GCThreadLock();

    /**
     * Unblocks the collector from running. 
     */
    ~GCThreadLock();

    GCThreadLock(const GCThreadLock&) = delete;
    GCThreadLock(GCThreadLock&&) = delete;
};


#endif //GCLIB_GCTHREADLOCK_HPP
