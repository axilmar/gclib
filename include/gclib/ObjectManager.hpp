#ifndef GCLIB_OBJECTMANAGER_HPP
#define GCLIB_OBJECTMANAGER_HPP


#include "IObjectManager.hpp"


namespace gclib {


    template <class T> class ObjectManager : public IObjectManager {
    public:
        void finalize(void* const begin, void* const end) final {
            reinterpret_cast<T*>(begin)->~T();
        }
    };


} //namespace gclib


#endif //GCLIB_OBJECTMANAGER_HPP
