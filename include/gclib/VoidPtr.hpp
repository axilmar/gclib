#ifndef GCLIB_VOIDPTR_HPP
#define GCLIB_VOIDPTR_HPP


namespace gclib
{


    /**
        Class the represents a pointer to void.
        It knows how to perform the basic actions of construction and assignment
        in a collector-safe way.
     */
    class VoidPtr
    {
    protected:
        /**
            The constructor.
            Copies the given value in a collector non-safe way.
            @param value initial value.
         */
        VoidPtr(void* value = nullptr) noexcept : m_value(value)
        {
        }

        /**
            Copies a pointer value in a collector non-safe way.
            @param ptr source object.
         */
        VoidPtr(const VoidPtr& ptr) noexcept : m_value(ptr.m_value)
        {
        }

        /**
            Deleted because subclasses need to implement the move in a collector-safe way.
            @param ptr source object.
         */
        VoidPtr(VoidPtr&& ptr) noexcept = delete;

        /**
            Assigns a new value in a collector-safe manner.
            @param value new value.
            @return reference to this.
         */
        VoidPtr& operator = (void* value) noexcept;

        /**
            Copies a pointer in a collector-safe manner.
            @param ptr source object.
            @return reference to this.
         */
        VoidPtr& operator = (const VoidPtr& ptr) noexcept;

        /**
            Moves a pointer in a collector-safe manner.
            @param ptr source object.
            @return reference to this.
         */
        VoidPtr& operator = (VoidPtr&& ptr) noexcept;

        /**
            Returns the raw pointer value.
            @return the raw pointer value.
         */
        void* get() const
        {
            return m_value;
        }

        /**
            Sets the pointer to null in a collector-non-safe manner (i.e. not synchronized).
         */
        void reset() noexcept
        {
            m_value = nullptr;
        }

        /**
            Allows the reset of the given ptr.
            @param ptr ptr to reset.
         */
        static void reset(VoidPtr& ptr) noexcept
        {
            ptr.reset();
        }

    private:
        void* m_value;
    };


} //namespace gclib


#endif //GCLIB_VOIDPTR_HPP
