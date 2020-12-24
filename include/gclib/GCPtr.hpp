#ifndef GCLIB_GCPTR_HPP
#define GCLIB_GCPTR_HPP


#include <type_traits>
#include "GCPtrStruct.hpp"


///private GC ptr functions.
class GCPtrPriv {
private:
    //init ptr, copy source value
    static void initCopy(GCPtrStruct* ptr, void* src);

    //init ptr, move source value
    static void initMove(GCPtrStruct* ptr, void*& src);

    //remove ptr from collector
    static void cleanup(GCPtrStruct* ptr);

    //copy ptr value.
    static void copy(void*& dst, void* src);

    //move ptr value.
    static void move(void*& dst, void*& src);

    template <class T> friend class GCPtr;
};


/**
 * A garbage collected pointer.
 * @param T type of value to point to.
 */
template <class T> class GCPtr : private GCPtrStruct {
public:
    /**
     * The default constructor.
     * @param value initial value.
     */
    GCPtr(T* value = nullptr) {
        GCPtrPriv::initCopy(this, value);
    }

    /**
     * The copy constructor.
     * @param ptr source object.
     */
    GCPtr(const GCPtr& ptr) {
        GCPtrPriv::initCopy(this, ptr.value);
    }

    /**
     * The move constructor.
     * @param ptr source object.
     */
    GCPtr(GCPtr&& ptr) {
        GCPtrPriv::initMove(this, ptr.value);
    }

    /**
     * The copy constructor from subtype.
     * @param ptr source object.
     */
    template <class U, class = std::enable_if_t<std::is_base_of_v<T, U>, int>>
    GCPtr(const GCPtr<U>& ptr) {
        GCPtrPriv::initCopy(this, ptr.value);
    }

    /**
     * The move constructor from subtype.
     * @param ptr source object.
     */
    template <class U, class = std::enable_if_t<std::is_base_of_v<T, U>, int>>
    GCPtr(GCPtr<U>&& ptr) {
        GCPtrPriv::initMove(this, ptr.value);
    }

    /**
     * The destructor.
     */
    ~GCPtr() {
        GCPtrPriv::cleanup(this);
    }

    /**
     * Assignment from raw value.
     * @param value value.
     * @return reference to this.
     */
    GCPtr& operator = (T* value) {
        GCPtrPriv::copy(this->value, value);
        return *this;
    }

    /**
     * Copy assignment.
     * @param ptr source object.
     * @return reference to this.
     */
    GCPtr& operator = (const GCPtr& ptr) {
        GCPtrPriv::copy(value, ptr.value);
        return *this;
    }

    /**
     * Move assignment.
     * @param ptr source object.
     * @return reference to this.
     */
    GCPtr& operator = (GCPtr&& ptr) {
        GCPtrPriv::move(value, ptr.value);
        return *this;
    }

    /**
     * Copy assignment from subtype.
     * @param ptr source object.
     * @return reference to this.
     */
    template <class U, class = std::enable_if_t<std::is_base_of_v<T, U>, int>>
    GCPtr& operator = (const GCPtr<U>& ptr) {
        GCPtrPriv::copy(value, ptr.value);
        return *this;
    }

    /**
     * Move assignment from subtype.
     * @param ptr source object.
     * @return reference to this.
     */
    template <class U, class = std::enable_if_t<std::is_base_of_v<T, U>, int>>
    GCPtr& operator = (GCPtr<U>&& ptr) {
        GCPtrPriv::move(value, ptr.value);
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

private:
    template <class U> friend class GCPtr;
};


#endif //GCLIB_GCPTR_HPP
