#ifndef GCLIB_MAKE_HPP
#define GCLIB_MAKE_HPP


#include <new>
#include "VoidPtr.hpp"
#include "ObjectManager.hpp"


namespace gclib {


    struct Make {
    private:
        static VoidPtr allocate(const std::size_t size, IObjectManager* om);
    };


    template <class T, class... Args> VoidPtr make(Args&&... args) {
        static ObjectManager<T> om;
        VoidPtr mem = Make::allocate(sizeof(T), &om);
        return new (mem) T(std::forward<Args>(args)...);
    }


} //namespace gclib


#endif //GCLIB_MAKE_HPP
