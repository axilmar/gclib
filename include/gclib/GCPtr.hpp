#ifndef GCLIB_GCPTR_HPP
#define GCLIB_GCPTR_HPP


#include <type_traits>
#include <memory>
#include "GCPtrStruct.hpp"
#include "GCPtrOperations.hpp"
#include "GCDeleteOperations.hpp"


///private GC ptr functions.
class GCPtrPrivate {
private:
    //init ptr, copy source value
    static void initCopy(GCPtrStruct* ptr, void* src);

    //init ptr, move source value
    static void initMove(GCPtrStruct* ptr, void*& src);

    //remove ptr from collector
    static void cleanup(GCPtrStruct* ptr);

    template <class T> friend class GCPtr;
};


/**
 * A garbage collected pointer.
 * It is a 'fat' pointer class: it contains extra members so as that it can attach itself
 * to the appropriate list of pointers (either the root set or the set of the block currently
 * being initialized.
 * 
 * For a non-fat version of a garbage-collected pointer, the class GCBasicPtr provides
 * garbage-collection functionality but it must be manually visited during collection time.
 * 
 * @param T type of value to point to.
 */
template <class T> class GCPtr : private GCPtrStruct {
public:
    /**
     * The default constructor.
     * @param value initial value.
     */
    GCPtr(T* value = nullptr) {
        GCPtrPrivate::initCopy(this, value);
    }

    /**
     * The copy constructor.
     * @param ptr source object.
     */
    GCPtr(const GCPtr& ptr) {
        GCPtrPrivate::initCopy(this, ptr.value);
    }

    /**
     * The move constructor.
     * @param ptr source object.
     */
    GCPtr(GCPtr&& ptr) {
        GCPtrPrivate::initMove(this, ptr.value);
    }

    /**
     * The copy constructor from subtype.
     * @param ptr source object.
     */
    template <class U, class = std::enable_if_t<std::is_base_of_v<T, U>, int>>
    GCPtr(const GCPtr<U>& ptr) {
        GCPtrPrivate::initCopy(this, ptr.value);
    }

    /**
     * The move constructor from subtype.
     * @param ptr source object.
     */
    template <class U, class = std::enable_if_t<std::is_base_of_v<T, U>, int>>
    GCPtr(GCPtr<U>&& ptr) {
        GCPtrPrivate::initMove(this, ptr.value);
    }

    /**
     * The destructor.
     */
    ~GCPtr() {
        GCPtrPrivate::cleanup(this);
    }

    /**
     * Assignment from raw value.
     * @param value value.
     * @return reference to this.
     */
    GCPtr& operator = (T* value) {
        GCPtrOperations::copy(this->value, value);
        return *this;
    }

    /**
     * Copy assignment.
     * @param ptr source object.
     * @return reference to this.
     */
    GCPtr& operator = (const GCPtr& ptr) {
        GCPtrOperations::copy(value, ptr.value);
        return *this;
    }

    /**
     * Move assignment.
     * @param ptr source object.
     * @return reference to this.
     */
    GCPtr& operator = (GCPtr&& ptr) {
        GCPtrOperations::move(value, ptr.value);
        return *this;
    }

    /**
     * Copy assignment from subtype.
     * @param ptr source object.
     * @return reference to this.
     */
    template <class U, class = std::enable_if_t<std::is_base_of_v<T, U>, int>>
    GCPtr& operator = (const GCPtr<U>& ptr) {
        GCPtrOperations::copy(value, ptr.value);
        return *this;
    }

    /**
     * Move assignment from subtype.
     * @param ptr source object.
     * @return reference to this.
     */
    template <class U, class = std::enable_if_t<std::is_base_of_v<T, U>, int>>
    GCPtr& operator = (GCPtr<U>&& ptr) {
        GCPtrOperations::move(value, ptr.value);
        return *this;
    }

    /**
     * Returns the raw pointer value.
     * @return the raw pointer value.
     */
    T* get() const noexcept {
        return reinterpret_cast<T*>(value);
    }

    /**
     * Auto conversion to raw pointer value.
     * @return the raw pointer value.
     */
    operator T*() const noexcept {
        return get();
    }

    /**
     * Auto conversion to shared ptr.
     * @return shared ptr to this; will apply reference counting to this object.
     */
    operator std::shared_ptr<T>() const noexcept {
        return std::shared_ptr<T>{ get(), GCDeleteOperations::gcdelete };
    }

    /**
     * The dereference operator.
     * @return the raw pointer value.
     * @exception std::runtime_error thrown if the pointer is null.
     */
    T& operator *() const {
        return value ? *get() : throw std::runtime_error("null ptr exception");
    }

    /**
     * Member access.
     * @return the raw pointer value.
     * @exception std::runtime_error thrown if the pointer is null.
     */
    T* operator ->() const {
        return value ? get() : throw std::runtime_error("null ptr exception");
    }

    /**
     * Returns a new pointer with a negative offset.
     * @param off offset.
     * @return new pointer with the given offset.
     */
    template <class N> GCPtr operator - (N off) const {
        return get() - off;
    }

    /**
     * Returns a new pointer with a positive offset.
     * @param off offset.
     * @return new pointer with the given offset.
     */
    template <class N> GCPtr operator + (N off) const {
        return get() + off;
    }

    /**
     * Offsets the pointer negatively by the given value.
     * @para off offset.
     * @return reference to this.
     */
    template <class N> GCPtr& operator -= (N off) {
        GCPtrOperations::copy(value, get() - off);
        return *this;
    }

    /**
     * Offsets the pointer positively by the given value.
     * @para off offset.
     * @return reference to this.
     */
    template <class N> GCPtr& operator += (N off) {
        GCPtrOperations::copy(value, get() + off);
        return *this;
    }

    /**
     * Sets this pointer to null.
     * @return previous pointer value.
     */
    T* reset() {
        T* result = get();
        operator = (nullptr);
        return result;
    }

private:
    template <class U> friend class GCPtr;
};


#endif //GCLIB_GCPTR_HPP
