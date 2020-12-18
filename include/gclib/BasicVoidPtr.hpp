#ifndef GCLIB_BASICVOIDPTR_HPP
#define GCLIB_BASICVOIDPTR_HPP


namespace gclib {


    /**
        Base class for garbage-collected pointers.        
        It can point to any object, either garbage-collected or not.
        It can point to the middle of an object or array.
     */
    class BasicVoidPtr {
    protected:
        /**
            The pointer's value.
            Protected so as that subclasses can manipulate it as needed.
         */
        void* m_value;

        /**
            Protected constructor; only subclasses can create instances.
         */
        BasicVoidPtr() {
        }

        /**
            Protected copy constructor; only subclasses can create instances.
            @param ptr source object.
         */
        BasicVoidPtr(const BasicVoidPtr& ptr) {
        }

        /**
            Protected move constructor; only subclasses can create instances.
            @param ptr source object.
         */
        BasicVoidPtr(BasicVoidPtr&& ptr) {
        }

        /**
            Protected constructor; only subclasses can destroy instances.
         */
        ~BasicVoidPtr() {
        }

        /**
            Assignment from value.
            @param value new value.
            @return reference to this.
         */
        BasicVoidPtr& operator = (void* value);

        /**
            The copy assignment operator.
            @param ptr source object.
            @return reference to this.
         */
        BasicVoidPtr& operator = (const BasicVoidPtr& ptr);

        /**
            The move assignment operator.
            @param ptr source object.
            @return reference to this.
         */
        BasicVoidPtr& operator = (BasicVoidPtr&& ptr);
    };


} //namespace gclib


#endif //GCLIB_BASICVOIDPTR_HPP
