#ifndef GCLIB_VOIDPTR_HPP
#define GCLIB_VOIDPTR_HPP


#include "BasicVoidPtr.hpp"
#include "DNode.hpp"


namespace gclib {


    /**
        A pointer class that is registered to the collector while in scope,
        so as that the collector automatically knows where pointers are.
     */
    class VoidPtr : public BasicVoidPtr, public DNode<VoidPtr> {
    protected:
        /**
            The default constructor.
            The pointer is added to the collector.
            @param value initial value.
         */
        VoidPtr(void* value = nullptr);

        /**
            The copy constructor.
            The pointer is added to the collector.
            @param ptr source object.
         */
        VoidPtr(const VoidPtr& ptr);

        /**
            The move constructor.
            The pointer is added to the collector.
            @param ptr source object.
         */
        VoidPtr(VoidPtr&& ptr);

        /**
            The destructor.
            The pointer is removed from the collector.
         */
        ~VoidPtr();

        /**
            Assignment from value.
            @param value new value.
            @return reference to this.
         */
        VoidPtr& operator = (void* value);

        /**
            The copy assignment operator.
            @param ptr source object.
            @return reference to this.
         */
        VoidPtr& operator = (const VoidPtr& ptr);

        /**
            The move assignment operator.
            @param ptr source object.
            @return reference to this.
         */
        VoidPtr& operator = (VoidPtr&& ptr);

    private:
        //mutex to use for synchronization
        class Mutex& m_mutex;
    };


} //namespace gclib


#endif //GCLIB_VOIDPTR_HPP
