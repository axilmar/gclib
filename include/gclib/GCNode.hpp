#ifndef GCLIB_GCNODE_HPP
#define GCLIB_GCNODE_HPP


/**
 * Base class for objects that are to be added to doubly-linked lists.
 * @param T type of class derived from this class.
 */
template <class T> struct GCNode {
    ///pointer to previous object.
    T* prev;

    ///pointer to next object.
    T* next;
    
    /**
     * Removes the node from its list.
     * No provision is made about if prev/next links are valid.
     */
    void detach() noexcept {
        prev->next = next;
        next->prev = prev;
    }
};


#endif //GCLIB_GCNODE_HPP
