#ifndef GCLIB_STATICVOIDPTR_HPP
#define GCLIB_STATICVOIDPTR_HPP


#include "BasicVoidPtr.hpp"


namespace gclib {


    /**
        A garbage-collected pointer that must be manually tracked.
        Provided as a more lightweight alternative for member pointers.
     */
    class StaticVoidPtr : public BasicVoidPtr {
    public:
    protected:
        /**
            The default constructor.
            @param value initial value.
         */
        StaticVoidPtr(void* value = nullptr);

        /**
            The copy constructor.
            @param ptr source object.
         */
        StaticVoidPtr(const StaticVoidPtr& ptr);

        /**
            The move constructor.
            @param ptr source object.
         */
        StaticVoidPtr(StaticVoidPtr&& ptr);

        using BasicVoidPtr::operator =;
    };


} //namespace gclib


#endif //GCLIB_STATICVOIDPTR_HPP
