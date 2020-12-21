#ifndef GCLIB_MAKE_HPP
#define GCLIB_MAKE_HPP


#include <new>
#include "VoidPtr.hpp"
#include "ObjectManager.hpp"


namespace gclib {


    struct Make {
    private:
        static VoidPtr allocate(const std::size_t size, IObjectManager* om);
        static void deallocate(void* const mem);
    };


    template <class T, class... Args> VoidPtr make(Args&&... args) {
        static ObjectManager<T> om;
        VoidPtr mem = Make::allocate(sizeof(T), &om);
        try {
            return new (mem) T(std::forward<Args>(args)...);
        }
        catch (...) {
            Make::deallocate(std::move(mem));
            throw;
        }
        return nullptr;
    }


} //namespace gclib


#endif //GCLIB_MAKE_HPP
