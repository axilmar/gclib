#ifndef GCLIB_STACKVOIDPTR_HPP
#define GCLIB_STACKVOIDPTR_HPP


#include <utility>
#include "SinglyLinkedListNode.hpp"
#include "VoidPtr.hpp"


namespace gclib
{


    /**
        Base class for stack void pointers.
     */
    class StackVoidPtr : public SinglyLinkedListNode<StackVoidPtr>, public VoidPtr
    {
    protected:
        /**
            The constructor from value.
            It writes this stack pointer into a thread-local singly-linked list in a collector-safe manner.
            @param value initial value.
         */
        StackVoidPtr(void* value = nullptr);

        /**
            The copy constructor from void ptr.
            It writes this stack pointer into a thread-local singly-linked list in a collector-safe manner.
            @param ptr source object.
         */
        StackVoidPtr(const VoidPtr& ptr);

        /**
            The move constructor from void ptr.
            It writes this stack pointer into a thread-local singly-linked list in a collector-safe manner.
            @param ptr source object.
         */
        StackVoidPtr(VoidPtr&& ptr);

        /**
            The copy constructor from void ptr.
            It writes this stack pointer into a thread-local singly-linked list in a collector-safe manner.
            @param ptr source object.
         */
        StackVoidPtr(const StackVoidPtr& ptr) : StackVoidPtr(static_cast<const VoidPtr&>(ptr))
        {
        }

        /**
            The move constructor from ptr.
            It writes this stack pointer into a thread-local singly-linked list in a collector-safe manner.
            @param ptr source object.
         */
        StackVoidPtr(StackVoidPtr&& ptr) : StackVoidPtr(std::move(static_cast<VoidPtr&>(ptr)))
        {
        }

        /**
            The destructor.
            It unregisters the pointer from the collector in a collector-safe manner.
         */
        ~StackVoidPtr();

    private:
        friend class SinglyLinkedList<StackVoidPtr>;
    };


} //namespace gclib


#endif //GCLIB_STACKVOIDPTR_HPP
