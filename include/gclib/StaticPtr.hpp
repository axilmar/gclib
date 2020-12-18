#ifndef GCLIB_STATICPTR_HPP
#define GCLIB_STATICPTR_HPP


#include "TypedPtr.hpp"
#include "IPtrManager.hpp"


namespace gclib {


    /**
        Definition of a pointer type that is managed manually with regard to the collector.
     */
    template <class T> using StaticPtr = TypedPtr<T, StaticVoidPtr>;


    /**
        Object manager for a static pointer.
     */
    template <class T> class ObjectManager<StaticPtr<T>> : public IObjectManager {
    public:
        /**
            Processes the given ptr with the given ptr manager.
            @param begin start of memory.
            @param end end of memory.
            @param pm pointer manager.
         */
        void scanPtrs(void* begin, void* end, IPtrManager& pm) noexcept final {
            pm.process(*(StaticPtr<T>*)begin);
        }

        /**
            Empty; there is no finalizer for static ptrs.
            @param begin start of memory.
            @param end end of memory.
         */
        void finalize(void* begin, void* end) noexcept final {
        }
    };


    /**
        Object manager for a static pointer array.
     */
    template <class T> class ObjectManager<StaticPtr<T>[]> : public IObjectManager {
    public:
        /**
            Processes the given ptr array with the given ptr manager.
            @param begin start of memory.
            @param end end of memory.
            @param pm pointer manager.
         */
        void scanPtrs(void* begin, void* end, IPtrManager& pm) noexcept final {
            for (StaticPtr<T>* ptr = (StaticPtr<T>*)begin; ptr < end; ++ptr) {
                pm.process(*ptr);
            }
        }

        /**
            Empty; there is no finalizer for static ptrs.
            @param begin start of memory.
            @param end end of memory.
         */
        void finalize(void* begin, void* end) noexcept final {
        }
    };


} //namespace gclib


#endif //GCLIB_STATICPTR_HPP
