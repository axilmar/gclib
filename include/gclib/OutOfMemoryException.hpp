#ifndef GCLIB_OUTOFMEMORYEXCEPTION_HPP
#define GCLIB_OUTOFMEMORYEXCEPTION_HPP


#include <stdexcept>


namespace gclib {


    /**
        Exception thrown when there is no more memory in the collector.
     */
    class OutOfMemoryException : public std::bad_alloc {
    public:
        /**
            Returns the explanatory message.
            @return the explanatory message.
         */
        const char* what() const noexcept final {
            return "Out of garbage-collected memory.";
        }
    };


} //namespace gclib


#endif //GCLIB_OUTOFMEMORYEXCEPTION_HPP
