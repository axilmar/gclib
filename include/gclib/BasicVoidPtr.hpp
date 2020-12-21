#ifndef GCLIB_BASICVOIDPTR_HPP
#define GCLIB_BASICVOIDPTR_HPP


namespace gclib {


    class BasicVoidPtr {
    public:
        BasicVoidPtr(void* value = nullptr) noexcept : m_value(value) {
        }

        BasicVoidPtr(const BasicVoidPtr& ptr) noexcept : m_value(ptr.m_value) {
        }

        BasicVoidPtr(BasicVoidPtr&& ptr) noexcept;

        BasicVoidPtr& operator =(void* value) noexcept;

        BasicVoidPtr& operator =(const BasicVoidPtr& ptr) noexcept;

        BasicVoidPtr& operator =(BasicVoidPtr&& ptr) noexcept;

        void* get() const noexcept {
            return m_value;
        }

        operator void*() const noexcept {
            return get();
        }

    private:
        void* m_value;
        friend class VoidPtr;
    };


} //namespace gclib


#endif //GCLIB_BASICVOIDPTR_HPP
