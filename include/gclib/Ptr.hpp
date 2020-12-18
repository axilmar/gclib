#ifndef GCLIB_PTR_HPP
#define GCLIB_PTR_HPP


#include "TypedPtr.hpp"
#include "VoidPtr.hpp"


namespace gclib {


    /**
        Definition of a pointer type that is managed dynamically by the collector.
     */
    template <class T> using Ptr = TypedPtr<T, VoidPtr>;


} //namespace gclib


#endif //GCLIB_PTR_HPP
