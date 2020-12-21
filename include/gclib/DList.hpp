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
            init();
        }

        /**
            The object is not copyable.
         */
        DList(const DList&) = delete;

        /**
            Moves the contents of the given list to this list.
            @param list list to move.
         */
        DList(DList&& list) {
            move(std::move(list));
        }

        /**
            The object is not copyable.
         */
        DList& operator = (const DList&) = delete;

        /**
            Moves the contents of the given list to this list.
            @param list list to move.
         */
        DList& operator = (DList&& list) {
            move(std::move(list));
            return *this;
        }

        /**
            Checks if the list is empty.
            @return true if empty, false otherwise.
         */
        bool empty() const noexcept {
            return m_head.m_prev == m_head.m_next;
        }

        /**
            Checks if the list is not empty.
            @return true if not empty, false otherwise.
         */
        bool notEmpty() const noexcept {
            return m_head.m_prev != m_head.m_next;
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
            Iterates the list, passing each item to the given function.
            @param func function to invoke.
         */
        template <class F> void forEach(F&& func) const {
            for (T* object = first(); object != end(); object = object->next()) {
                func(object);
            }
        }

        /**
            Prepends an item.
            No provision is made to if the item belongs in another list.
            @param item item to prepend.
         */
        void prepend(T* item) noexcept {
            DNode<T>* const node = item;
            node->m_prev = head();
            node->m_next = m_head.m_next;
            m_head.m_next->m_prev = item;
            m_head.m_next = item;
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

        //initializes the list
        void init() {
            m_head.m_prev = m_head.m_next = head();
        }

        //moves the given list to this
        void move(DList&& list) {
            if (list.empty()) {
                init();
            }
            else {
                list.m_head.m_next->m_prev = head();
                list.m_head.m_prev->m_next = head();
                m_head.m_next = list.m_head.m_next;
                m_head.m_prev = list.m_head.m_prev;
            }
        }
    };


} //namespace gclib


#endif //GCLIB_DLIST_HPP
