#ifndef GCLIB_VOIDPTR_HPP
#define GCLIB_VOIDPTR_HPP


#include <utility>
#include "BasicVoidPtr.hpp"
#include "DNode.hpp"


namespace gclib {


    template <class T> class DList;


    class VoidPtr : public BasicVoidPtr, private DNode<VoidPtr> {
    public:
        VoidPtr(void* value = nullptr) noexcept;

        VoidPtr(const VoidPtr& ptr) noexcept;

        VoidPtr(VoidPtr&& ptr) noexcept;

        ~VoidPtr();

        VoidPtr& operator = (void* value) noexcept {
            BasicVoidPtr::operator = (value);
            return *this;
        }

        VoidPtr& operator = (const VoidPtr& ptr) noexcept {
            BasicVoidPtr::operator = (ptr);
            return *this;
        }

        VoidPtr& operator = (VoidPtr&& ptr) noexcept {
            BasicVoidPtr::operator = (std::move(ptr));
            return *this;
        }

        void* get() const noexcept {
            return BasicVoidPtr::get();
        }

        operator void*() const noexcept {
            return get();
        }

    private:
        struct ThreadData* m_owner;

        friend class DNode<VoidPtr>;
        friend class DList<VoidPtr>;
    };


} //namespace gclib


#endif //GCLIB_VOIDPTR_HPP
