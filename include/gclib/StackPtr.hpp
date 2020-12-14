#ifndef GCLIB_STACKPTR_HPP
#define GCLIB_STACKPTR_HPP


#include "StackVoidPtr.hpp"
#include "Ptr.hpp"


namespace gclib
{


    /**
        A stack pointer.
        @param T type of object to point to.
     */
    template <class T> class StackPtr : public Ptr<T, StackVoidPtr>
    {
    public:
        using Ptr<T, StackVoidPtr>::Ptr;
        using Ptr<T, StackVoidPtr>::operator =;
    };


} //namespace gclib


#endif //GCLIB_STACKPTR_HPP
