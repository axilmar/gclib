#ifndef GCLIB_DNODE_HPP
#define GCLIB_DNODE_HPP


namespace gclib {


    template <class T> class DList;


    /**
        Base class for objects that can be inserted into double-linked lists.
        @param T type of object that is derived from this node.
     */
    template <class T> class DNode {
    public:
        /**
            Returns pointer to previous node.
            @return pointer to previous node.
         */
        T* prev() const noexcept {
            return m_prev;
        }

        /**
            Returns pointer to next node.
            @return pointer to next node.
         */
        T* next() const noexcept {
            return m_next;
        }

        /**
            Removes this node from its list.
         */
        void detach() noexcept {
            m_prev->m_next = m_next;
            m_next->m_prev = m_prev;
        }

    private:
        T* m_prev;
        T* m_next;

        friend class DList<T>;
    };


} //namespace gclib


#endif //GCLIB_DNODE_HPP
