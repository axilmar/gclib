#ifndef GCLIB_STATICPTR_HPP
#define GCLIB_STATICPTR_HPP


#include "TypedPtr.hpp"
#include "StaticVoidPtr.hpp"


namespace gclib {


    /**
        Definition of a pointer type that is managed manually in regard to the collector.
     */
    template <class T> using StaticPtr = TypedPtr<T, StaticVoidPtr>;


} //namespace gclib


#endif //GCLIB_STATICPTR_HPP
