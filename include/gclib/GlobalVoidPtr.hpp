#ifndef GCLIB_GLOBALVOIDPTR_HPP
#define GCLIB_GLOBALVOIDPTR_HPP


#include <utility>
#include "DoublyLinkedListNode.hpp"
#include "VoidPtr.hpp"


namespace gclib
{


    /**
        Base class for global void pointers.
     */
    class GlobalVoidPtr : public DoublyLinkedListNode<GlobalVoidPtr>, public VoidPtr
    {
    protected:
        /**
            The constructor from value.
            It writes this global pointer into a thread-local but heap-allocated doubly-linked list in a collector-safe manner.
            @param value initial value.
         */
        GlobalVoidPtr(void* value = nullptr);

        /**
            The copy constructor from void ptr.
            It writes this global pointer into a thread-local but heap-allocated doubly-linked list in a collector-safe manner.
            @param ptr source object.
         */
        GlobalVoidPtr(const VoidPtr& ptr);

        /**
            The move constructor from void ptr.
            It writes this global pointer into a thread-local but heap-allocated doubly-linked list in a collector-safe manner.
            @param ptr source object.
         */
        GlobalVoidPtr(VoidPtr&& ptr);

        /**
            The copy constructor from void ptr.
            It writes this global pointer into a thread-local but heap-allocated doubly-linked list in a collector-safe manner.
            @param ptr source object.
         */
        GlobalVoidPtr(const GlobalVoidPtr& ptr) : GlobalVoidPtr(static_cast<const VoidPtr&>(ptr))
        {
        }

        /**
            The move constructor from ptr.
            It writes this global pointer into a thread-local but heap-allocated doubly-linked list in a collector-safe manner.
            @param ptr source object.
         */
        GlobalVoidPtr(GlobalVoidPtr&& ptr) : GlobalVoidPtr(std::move(static_cast<VoidPtr&>(ptr)))
        {
        }

        /**
            The destructor.
            It unregisters the pointer from the collector in a collector-safe manner.
         */
        ~GlobalVoidPtr();

    private:
        friend class DoublyLinkedList<GlobalVoidPtr>;
        class ThreadData* m_threadData;
    };


} //namespace gclib


#endif //GCLIB_GLOBALVOIDPTR_HPP
