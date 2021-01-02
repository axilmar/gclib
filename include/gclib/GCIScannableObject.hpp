#ifndef GCLIB_GCISCANNABLEOBJECT_HPP
#define GCLIB_GCISCANNABLEOBJECT_HPP


/**
 * Interface for objects that are manually scanned. 
 */
class GCIScannableObject {
public:
    /**
     * The scan function.
     * Subclasses must provide the code to manually scan the object for pointers.
     */
    virtual void scan() const noexcept = 0 {
    }
};


#endif //GCLIB_GCISCANNABLEOBJECT_HPP
