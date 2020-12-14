#ifndef GCLIB_DOUBLYLINKEDLISTNODE_HPP
#define GCLIB_DOUBLYLINKEDLISTNODE_HPP


namespace gclib
{

    template <class T> class DoublyLinkedList;


    /**
        Base class for doubly-linked list nodes.
        @param T the derived class.
     */
    template <class T> class DoublyLinkedListNode
    {
    public:
        /**
            Returns pointer to previous node.
            @return pointer to previous node.
         */
        T* getPrev() const noexcept
        {
            return m_prev;
        }

        /**
            Returns pointer to next node.
            @return pointer to next node.
         */
        T* getNext() const noexcept
        {
            return m_next;
        }

    protected:
        /**
            Removes the node from its lisst.
         */
        void detach() noexcept
        {
            m_prev->m_next = m_next;
            m_next->m_prev = m_prev;
        }

    private:
        T* m_prev;
        T* m_next;

        friend class DoublyLinkedList<T>;
    };


} //namespace gclib


#endif //GCLIB_DOUBLYLINKEDLISTNODE_HPP
