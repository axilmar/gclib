#ifndef GCLIB_OBJECTMANAGER_HPP
#define GCLIB_OBJECTMANAGER_HPP


#include <type_traits>
#include "IObjectManager.hpp"
#include "IPtrManager.hpp"


namespace gclib {


    /**
        Checks if a type has the method 'scanPtrs'.
        @param T type of check if it has the method 'scanPtrs'.
     */
    template <class T> class HasScanPtrsMethod {
        template <class T> static char test(decltype(&C::scanPtrs)) { return 0; }
        template <class T> static int test(...) { return 0; }
    public:
        /**
            Value; if true, then the given type has the scanPtrs method,
            otherwise it does not.
         */
        static constexpr bool Value = sizeof(test<T>(0)) == sizeof(char);
    };


    /**
        Object manager class.
        @param T type of object to manage.
     */
    template <class T> class ObjectManager : public IObjectManager {
    public:
        /**
            Invokes the relevant method of the given object, if the type implements the method 'scanPtrs(pm)'.
            @param begin start of memory.
            @param end end of memory.
            @param pm pointer manager.
         */
        void scanPtrs(void* begin, void* end, IPtrManager& pm) noexcept final {
            if constexpr (HasScanPtrsMethod<T>::Value) {
                static_cast<T*>(begin)->scanPtrs(pm);
            }
        }

        /**
            Invokes the object's destructor.
            @param begin start of memory.
            @param end end of memory.
         */
        void finalize(void* begin, void* end) noexcept final {
            static_cast<T*>(begin)->~T();
        }
    };


    /**
        Object manager class for array of objects.
        @param T type of object to manage.
     */
    template <class T> class ObjectManager<T[]> : public IObjectManager {
    public:
        /**
            Invokes the relevant method of the given objects, if the type implements the method 'scanPtrs(pm)'.
            @param begin start of memory.
            @param end end of memory.
            @param pm pointer manager.
         */
        void scanPtrs(void* begin, void* end, IPtrManager& pm) noexcept final {
            if constexpr (HasScanPtrsMethod<T>::Value) {
                for (T* obj = (T*)begin; obj < end; ++obj) {
                    obj->scanPtrs(pm);
                }
            }
        }

        /**
            Invokes the object's destructors, in reverse order.
            @param begin start of memory.
            @param end end of memory.
         */
        void finalize(void* begin, void* end) noexcept final {
            for (T* obj = (T*)end - 1; obj >= begin; --obj) {
                static_cast<T*>(obj)->~T();
            }
        }
    };


} //namespace gclib


#endif //GCLIB_OBJECTMANAGER_HPP
