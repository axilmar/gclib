#ifndef GCLIB_IPTRMANAGER_HPP
#define GCLIB_IPTRMANAGER_HPP


#include "StaticVoidPtr.hpp"


namespace gclib {


    /**
        Interface for processing the member pointers of an object.
     */
    class IPtrManager {
    public:
        /**
            Processes the given ptr.
            @param ptr ptr to process.
         */
        virtual void process(StaticVoidPtr& ptr) = 0;
    };


} //namespace gclib


#endif //GCLIB_IPTRMANAGER_HPP
