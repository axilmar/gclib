#ifndef GCLIB_TYPEDPTR_HPP
#define GCLIB_TYPEDPTR_HPP


#include <type_traits>
#include <stdexcept>
#include "ObjectManager.hpp"


namespace gclib {


    /**
        Typed pointer implementation.
        @param T type of object to point to.
        @param PtrImpl pointer implementation.
     */
    template <class T, class PtrImpl> class TypedPtr : public PtrImpl {
    public:
        /**
            The default constructor.
            @param value initial value.
         */
        TypedPtr(T* value = nullptr) : PtrImpl(value) {
        }

        /**
            The copy constructor.
            @param ptr source object.
         */
        TypedPtr(const TypedPtr& ptr) : PtrImpl(ptr) {
        }

        /**
            The move constructor.
            @param ptr source object.
         */
        TypedPtr(TypedPtr&& ptr) : PtrImpl(std::move(ptr)) {
        }

        /**
            The copy constructor from another pointer type.
            @param ptr source object.
         */
        template <class U, class I, typename = std::enable_if_t<std::is_base_of_v<T, U>, int>>
        TypedPtr(const TypedPtr<U, I>& ptr) : PtrImpl(ptr) {
        }

        /**
            The move constructor from another pointer type.
            @param ptr source object.
         */
        template <class U, class I, typename = std::enable_if_t<std::is_base_of_v<T, U>, int>>
        TypedPtr(TypedPtr<U, I>&& ptr) : PtrImpl(std::move(ptr)) {
        }

        /**
            Assignment from value.
            @param value new value.
            @return reference to this.
         */
        TypedPtr& operator = (void* value) {
            PtrImpl::operator = (value);
            return *this;
        }

        /**
            The copy assignment operator.
            @param ptr source object.
            @return reference to this.
         */
        TypedPtr& operator = (const TypedPtr& ptr) {
            PtrImpl::operator = (ptr);
            return *this;
        }

        /**
            The move assignment operator.
            @param ptr source object.
            @return reference to this.
         */
        TypedPtr& operator = (TypedPtr&& ptr) {
            PtrImpl::operator = (std::move(ptr));
            return *this;
        }

        /**
            The copy assignment operator from another pointer type.
            @param ptr source object.
            @return reference to this.
         */
        template <class U, class I, typename = std::enable_if_t<std::is_base_of_v<T, U>, int>>
        TypedPtr& operator = (const TypedPtr<U, I>& ptr) {
            PtrImpl::operator = (ptr);
            return *this;
        }

        /**
            The move assignment operator from another pointer type.
            @param ptr source object.
            @return reference to this.
         */
        template <class U, class I, typename = std::enable_if_t<std::is_base_of_v<T, U>, int>>
        TypedPtr& operator = (TypedPtr<U, I>&& ptr) {
            PtrImpl::operator = (std::move(ptr));
            return *this;
        }

        /**
            Returns a raw pointer.
            @return raw pointer.
         */
        T* get() const {
            return static_cast<T*>(PtrImpl::get());
        }

        /**
            The dereference operator.
            @return raw pointer.
            @exception std::runtime_error thrown when the pointer is null.
         */
        T& operator *() const {
            return get ? *get() : throw std::runtime_error("Null pointer exception");
        }

        /**
            Automatic conversion to raw pointer.
            @return raw pointer.
         */
        operator T* () const {
            return get();
        }

        /**
            Member access.
            @return raw pointer.
            @exception std::runtime_error thrown when the pointer is null.
         */
        T* operator ->() const {
            return get() ? get() : throw std::runtime_error("Null pointer exception");
        }

    private:
        template <class U, class I> friend class TypedPtr;
    };


} //namespace gclib


#endif //GCLIB_TYPEDPTR_HPP
