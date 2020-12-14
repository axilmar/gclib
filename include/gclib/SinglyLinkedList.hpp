#ifndef GCLIB_SINGLYLINKEDLIST_HPP
#define GCLIB_SINGLYLINKEDLIST_HPP


#include "SinglyLinkedListNode.hpp"


namespace gclib
{


    /**
        A singly-linked list.
        @param T type of item to store in the list.
     */
    template <class T> class SinglyLinkedList
    {
    public:
        /**
            Returns a pointer to the first element in the list.
            @return pointer to the first element in the list or the end of the list.
         */
        T* getFirst() const noexcept
        {
            return m_first;
        }

        /**
            Returns the end of the list.
            @return the end of the list.
         */
        const void* getEnd() const noexcept
        {
            return nullptr;
        }

        /**
            Prepends an object.
            @param object object to prepend.
         */
        void prepend(T* object) noexcept
        {
            prependNode(object);
        }

        /**
            Removes an object.
            @param object object to remove.
         */
        void remove(T* object) noexcept
        {
            removeNode(object);
        }

    private:
        T* m_first{ nullptr };

        //prepends a node
        void prependNode(SinglyLinkedListNode<T>* node) noexcept
        {
            node->m_next = m_first;
            m_first = static_cast<T*>(node);
        }

        //removes a node
        void removeNode(SinglyLinkedListNode<T>* node)
        {
            if (node == m_first)
            {
                m_first = m_first->m_next;
                return;
            }

            for (SinglyLinkedListNode<T> *prev = m_first; prev; prev = prev->m_next)
            {
                if (node == prev->m_next)
                {
                    prev->m_next = node->m_next;
                    return;
                }
            }
        }
    };


} //namespace gclib


#endif //GCLIB_SINGLYLINKEDLIST_HPP
