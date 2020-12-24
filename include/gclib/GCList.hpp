#ifndef GCLIB_GCLIST_HPP
#define GCLIB_GCLIST_HPP


#include "GCNode.hpp"


/**
 * A doubly-linked list.
 * @param T type of object to put in the list.
 */
template <class T> struct GCList : GCNode<T> {
    using GCNode<T>::prev;
    using GCNode<T>::next;

    /**
     * The default constructor.
     * It it set up so as that it points to itself.
     */
    GCList() noexcept {
        clear();
    }

    /**
     * The copy constructor.
     * It is deleted due to the presence of pointers.
     */
    GCList(const GCList&) = delete;

    /**
     * The move constructor.
     * The nodes of the given list are moved to this list.
     * @param list source list; emptied on return.
     */
    GCList(GCList&& list) noexcept {
        operator = (std::move(list));
    }

    /**
     * The copy assignment operator.
     * It is deleted due to the presence of pointers.
     */
    GCList& operator = (const GCList&) = delete;

    /**
     * The move assignment operator.
     * No provision is made if the given list is the same as this list.
     * @param list source list; emptied on return.
     * @return reference to this.
     */
    GCList& operator = (GCList&& list) noexcept {
        if (list.empty()) {
            clear();
        }
        else {
            list.prev->next = reinterpret_cast<T*>(static_cast<GCNode<T>*>(this));
            list.next->prev = reinterpret_cast<T*>(static_cast<GCNode<T>*>(this));
            next = list.next;
            prev = list.prev;
            list.clear();
        }
        return *this;
    }

    /**
     * checks if the list is empty.
     * @return true if empty, false otherwise.
     */
    bool empty() const noexcept {
        return next == end();
    }

    /**
     * Returns pointer to the first object.
     * @return pointer to the first object; if the list is empty, it returns end().
     */
    T* first() const noexcept {
        return next;
    }

    /**
     * Returns pointer to the last object.
     * @return pointer to the last object; if the list is empty, it returns end().
     */
    T* last() const noexcept {
        return prev;
    }

    /**
     * Returns pointer to the end of the list.
     * @return pointer to the end of the list.
     */
    const void* end() const noexcept {
        return reinterpret_cast<const T*>(static_cast<const GCNode<T>*>(this));
    }

    /**
     * Appends an item.
     * No provision is made if the node belongs to another list.
     * @param node nod to append.
     */
    void append(T* const node) noexcept {
        node->prev = prev;
        node->next = reinterpret_cast<T*>(static_cast<GCNode<T>*>(this));
        prev->next = node;
        prev = node;
    }

    /**
     * Appends the nodes of the given list to this list.
     * @param list source list; emptied.
     */
    void append(GCList<T>&& list) noexcept {
        if (list.empty()) return;
        list.next->prev = prev;
        list.prev->next = reinterpret_cast<T*>(static_cast<GCNode<T>*>(this));
        prev->next = list.next;
        prev = list.prev;
        list.clear();
    }

    /**
     * Sets up the list so as that it has no nodes.
     * Nodes are not detached, the list is setup to point to itself.
     */
    void clear() noexcept {
        prev = next = reinterpret_cast<T*>(static_cast<GCNode<T>*>(this));
    }
};


#endif //GCLIB_GCLIST_HPP
