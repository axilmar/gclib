#ifndef GCLIB_SINGLYLINKEDLISTNODE_HPP
#define GCLIB_SINGLYLINKEDLISTNODE_HPP


namespace gclib
{


    template <class T> class SinglyLinkedList;


    /**
        Base class for singly-linked list nodes.
     */
    template <class T> class SinglyLinkedListNode
    {
    public:
        /**
            Returns pointer to next node.
            @return pointer to next node.
         */
        T* getNext() const noexcept
        {
            return m_next;
        }

    private:
        T* m_next;

        friend class SinglyLinkedList<T>;
    };


} //namespace gclib


#endif //GCLIB_SINGLYLINKEDLISTNODE_HPP
