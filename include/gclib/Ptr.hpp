#ifndef GCLIB_PTR_HPP
#define GCLIB_PTR_HPP


#include <type_traits>


namespace gclib
{


    /**
        An implementation of a typed pointer based on an untyped pointer.
        @param T type of pointed object.
        @param Impl the pointer implementation.
     */
    template <class T, class Impl> class Ptr : public Impl
    {
    public:
        /**
            Constructor from value.
            @param value initial value.
         */
        Ptr(T* value = nullptr) : Impl(value)
        {
        }

        /**
            The copy constructor.
            @param ptr source object.
         */
        Ptr(const Ptr& ptr) : Impl(ptr)
        {
        }

        /**
            The move constructor.
            @param ptr source object.
         */
        Ptr(Ptr&& ptr) : Impl(std::move(ptr))
        {
        }

        /**
            The copy constructor from pointer to a derived type (and possibly of another implementation).
            @param ptr source object.
         */
        template <class OtherT, class OtherImpl, class = std::enable_if_t<std::is_base_of_v<T, OtherT>, int>>
        Ptr(const Ptr<OtherT, OtherImpl>& ptr) : Impl(ptr)
        {
        }

        /**
            The move constructor from pointer to a derived type (and possibly of another implementation).
            @param ptr source object.
         */
        template <class OtherT, class OtherImpl, class = std::enable_if_t<std::is_base_of_v<T, OtherT>, int>>
        Ptr(Ptr<OtherT, OtherImpl>&& ptr) : Impl(std::move(ptr))
        {
        }

        /**
            Assignment from value.
            @param value new value.
            @return reference to this.
         */
        Ptr& operator = (T* value)
        {
            Impl::operator = (value);
            return *this;
        }

        /**
            Copy assignment from ptr.
            @param ptr source object.
            @return reference to this.
         */
        Ptr& operator = (const Ptr& ptr)
        {
            Impl::operator = (ptr);
            return *this;
        }

        /**
            Move assignment from ptr.
            @param ptr source object.
            @return reference to this.
         */
        Ptr& operator = (Ptr&& ptr)
        {
            Impl::operator = (std::move(ptr));
            return *this;
        }

        /**
            Copy assignment from ptr.
            @param ptr source object.
            @return reference to this.
         */
        template <class OtherT, class OtherImpl, class = std::enable_if_t<std::is_base_of_v<T, OtherT>, int>>
        Ptr& operator = (const Ptr& ptr)
        {
            Impl::operator = (ptr);
            return *this;
        }

        /**
            Move assignment from ptr.
            @param ptr source object.
            @return reference to this.
         */
        template <class OtherT, class OtherImpl, class = std::enable_if_t<std::is_base_of_v<T, OtherT>, int>>
        Ptr& operator = (Ptr&& ptr)
        {
            Impl::operator = (std::move(ptr));
            return *this;
        }

        /**
            Returns the raw pointer value.
            @return the raw pointer value.
         */
        T* get() const
        {
            return static_cast<T*>(Impl::get());
        }

        /**
            Automatic conversion to raw pointer value.
            @return the raw pointer value.
         */
        operator T*() const
        {
            return get();
        }

        /**
            The dereference operator.
            @return reference to object.
            @exception std::runtime_error thrown if the pointer is null.
         */
        T& operator *() const
        {
            T* ptr = get();
            return ptr ? ptr : throw std::runtime_error("null pointer exception");
        }

        /**
            The member access operator.
            @return reference to object.
            @exception std::runtime_error thrown if the pointer is null.
         */
        T* operator ->() const
        {
            T* ptr = get();
            return ptr ? ptr : throw std::runtime_error("null pointer exception");
        }
    };


} //namespace gclib


#endif //GCLIB_PTR_HPP
