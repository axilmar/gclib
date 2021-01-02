#ifndef GCLIB_GCBASICPTR_HPP
#define GCLIB_GCBASICPTR_HPP


#include <stdexcept>
#include "GCPtrOperations.hpp"


/**
 * A pointer class that allows the manual management of ptr values.
 * 
 * Unlike the class GCBasicPtr, this is not a 'fat' pointer type: it has exactly
 * the same size as a raw pointer.
 * 
 * It can point to anything.
 * 
 * @param T type of object to point to.
 */
template <class T> class GCBasicPtr {
public:
    /**
     * The default constructor.
     * @param value initial value.
     */
    GCBasicPtr(T* value = nullptr) : m_value(value) {
    }

    /**
     * The copy constructor.
     * @param ptr source object.
     */
    GCBasicPtr(const GCBasicPtr& ptr) : m_value(ptr.m_value) {
    }

    /**
     * The move constructor.
     * @param ptr source object.
     */
    GCBasicPtr(GCBasicPtr&& ptr) : m_value(ptr.m_value) {
        GCPtrOperations::copy(ptr.m_value, nullptr);
    }

    /**
     * The copy constructor from subtype.
     * @param ptr source object.
     */
    template <class U, class = std::enable_if_t<std::is_base_of_v<T, U>, int>>
    GCBasicPtr(const GCBasicPtr<U>& ptr) : m_value(ptr.m_value) {
    }

    /**
     * The move constructor from subtype.
     * @param ptr source object.
     */
    template <class U, class = std::enable_if_t<std::is_base_of_v<T, U>, int>>
    GCBasicPtr(GCBasicPtr<U>&& ptr) : m_value(ptr.m_value) {
        GCPtrOperations::copy(ptr.m_value, nullptr);
    }

    /**
     * Assignment from raw value.
     * @param value value.
     * @return reference to this.
     */
    GCBasicPtr& operator = (T* value) {
        GCPtrOperations::copy(m_value, value);
        return *this;
    }

    /**
     * Copy assignment.
     * @param ptr source object.
     * @return reference to this.
     */
    GCBasicPtr& operator = (const GCBasicPtr& ptr) {
        GCPtrOperations::copy(m_value, ptr.m_value);
        return *this;
    }

    /**
     * Move assignment.
     * @param ptr source object.
     * @return reference to this.
     */
    GCBasicPtr& operator = (GCBasicPtr&& ptr) {
        GCPtrOperations::move(m_value, ptr.m_value);
        return *this;
    }

    /**
     * Copy assignment from subtype.
     * @param ptr source object.
     * @return reference to this.
     */
    template <class U, class = std::enable_if_t<std::is_base_of_v<T, U>, int>>
    GCBasicPtr& operator = (const GCBasicPtr<U>& ptr) {
        GCPtrOperations::copy(m_value, ptr.m_value);
        return *this;
    }

    /**
     * Move assignment from subtype.
     * @param ptr source object.
     * @return reference to this.
     */
    template <class U, class = std::enable_if_t<std::is_base_of_v<T, U>, int>>
    GCBasicPtr& operator = (GCBasicPtr<U>&& ptr) {
        GCPtrOperations::move(m_value, ptr.m_value);
        return *this;
    }

    /**
     * Returns the raw pointer value.
     * @return the raw pointer value.
     */
    T* get() const noexcept {
        return m_value;
    }

    /**
     * Auto conversion to raw pointer value.
     * @return the raw pointer value.
     */
    operator T*() const noexcept {
        return m_value;
    }

    /**
     * The dereference operator.
     * @return the raw pointer value.
     * @exception std::runtime_error thrown if the pointer is null.
     */
    T& operator *() const {
        return m_value ? *m_value : throw std::runtime_error("null ptr exception");
    }

    /**
     * Member access.
     * @return the raw pointer value.
     * @exception std::runtime_error thrown if the pointer is null.
     */
    T* operator ->() const {
        return m_value ? m_value : throw std::runtime_error("null ptr exception");
    }

    /**
     * Returns a new pointer with a negative offset.
     * @param off offset.
     * @return new pointer with the given offset.
     */
    template <class N> GCBasicPtr operator - (N off) const {
        return m_value - off;
    }

    /**
     * Returns a new pointer with a positive offset.
     * @param off offset.
     * @return new pointer with the given offset.
     */
    template <class N> GCBasicPtr operator + (N off) const {
        return m_value + off;
    }

    /**
     * Offsets the pointer negatively by the given value.
     * @para off offset.
     * @return reference to this.
     */
    template <class N> GCBasicPtr& operator -= (N off) {
        GCPtrOperations::copy(m_value, m_value - off);
        return *this;
    }

    /**
     * Offsets the pointer positively by the given value.
     * @para off offset.
     * @return reference to this.
     */
    template <class N> GCBasicPtr& operator += (N off) {
        GCPtrOperations::copy(m_value, m_value + off);
        return *this;
    }

    /**
     * Used for manually scanning the pointer during the mark phase of the collection. 
     */
    void scan() const noexcept {
        GCPtrOperations::scan(m_value);
    }

private:
    T* m_value;
    template <class U> friend class GCBasicPtr;
};


#endif //GCLIB_GCBASICPTR_HPP
