#ifndef GCLIB_DOUBLYLINKEDLIST_HPP
#define GCLIB_DOUBLYLINKEDLIST_HPP


#include "DoublyLinkedListNode.hpp"


namespace gclib
{


    /**
        A doubly-linked list.
        @param T type of item to store in the list.
     */
    template <class T> class DoublyLinkedList
    {
    public:
        /**
            Returns a pointer to the first element in the list.
            @return pointer to the first element in the list or the end of the list.
         */
        T* getFirst() const noexcept
        {
            return m_head.m_next;
        }

        /**
            Returns a pointer to the last element in the list.
            @return pointer to the last element in the list or the end of the list.
         */
        T* getLast() const noexcept
        {
            return m_head.m_prev;
        }

        /**
            Returns the end of the list.
            @return the end of the list.
         */
        const void* getEnd() const noexcept
        {
            return &m_head;
        }

        /**
            Appends an object.
            @param object object to append.
         */
        void append(T* object) noexcept
        {
            appendNode(object);
        }

    private:
        //list head
        struct Head
        {
            T* m_prev;
            T* m_next;
        } m_head{(T*)&m_head, (T*)&m_head};

        //appends node
        void appendNode(DoublyLinkedListNode<T>* node) noexcept
        {
            node->m_prev = m_head.m_prev;
            node->m_next = (T*)&m_head;
            m_head.m_prev->m_next = static_cast<T*>(node);
            m_head.m_prev = static_cast<T*>(node);
        }
    };


} //namespace gclib


#endif //GCLIB_DOUBLYLINKEDLIST_HPP
