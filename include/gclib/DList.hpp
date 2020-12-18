#ifndef GCLIB_DLIST_HPP
#define GCLIB_DLIST_HPP


#include "DNode.hpp"


namespace gclib {


    /**
        A circular, intrusive, doubly-linked list.
        @param T type of objects to store in the list.
     */
    template <class T> class DList {
    public:
        /**
            The default constructor.
            Initializes the list so as that first == last == list.
         */
        DList() noexcept {
            m_head.m_prev = m_head.m_next = head();
        }

        /**
            Checks if the list is empty.
            @return true if empty, false otherwise.
         */
        bool empty() const noexcept {
            return m_head.m_prev == m_head.m_next;
        }

        /**
            Returns pointer to the first item.
            @return pointer to the first item.
         */
        T* first() const noexcept {
            return m_head.m_next;
        }

        /**
            Returns pointer to the last item.
            @return pointer to the last item.
         */
        T* last() const noexcept {
            return m_head.m_prev;
        }

        /**
            Returns the list's end.
            @return the list's end.
         */
        const void* end() const noexcept {
            return head();
        }

        /**
            Appends an item.
            No provision is made to if the item belongs in another list.
            @param item item to append.
         */
        void append(T* item) noexcept {
            DNode<T>* const node = item;
            node->m_prev = m_head.m_prev;
            node->m_next = head();
            m_head.m_prev->m_next = item;
            m_head.m_prev = item;
        }

    private:
        //list head
        DNode<T> m_head;

        //returns the head item as const T*
        const T* head() const noexcept {
            return static_cast<const T*>(&m_head);
        }

        //returns the head item as T*
        T* head() noexcept {
            return static_cast<T*>(&m_head);
        }
    };


} //namespace gclib


#endif //GCLIB_DLIST_HPP
