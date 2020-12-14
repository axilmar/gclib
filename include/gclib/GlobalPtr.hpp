#ifndef GCLIB_GLOBALPTR_HPP
#define GCLIB_GLOBALPTR_HPP


#include "GlobalVoidPtr.hpp"
#include "Ptr.hpp"


namespace gclib
{


    /**
        A global pointer.
        It can function as a global variable, a member variable, or a stack variable.
        @param T type of object to point to.
     */
    template <class T> class GlobalPtr : public Ptr<T, GlobalVoidPtr>
    {
    public:
        using Ptr<T, GlobalVoidPtr>::Ptr;
        using Ptr<T, GlobalVoidPtr>::operator =;
    };


} //namespace gclib


#endif //GCLIB_GLOBALPTR_HPP
