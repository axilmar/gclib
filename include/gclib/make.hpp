#ifndef GCLIB_MAKE_HPP
#define GCLIB_MAKE_HPP


#include <new>
#include "Ptr.hpp"
#include "ObjectManager.hpp"


namespace gclib {


    //make private functions
    class MakePrivate {
    private:
        //locks the collector while an object is constructed

        //allocates a memory block from the collector
        static void* allocate(const std::size_t size, IObjectManager* om);

        //marks a block as invalid
        static void markInvalid(void* mem);

        //marks a block as valid
        static void markValid(void* mem);

        template <class T, class... Args> friend Ptr<T> make(Args&&... args);
    };


    /**
        Allocates memory for a garbage-collected object.
        @param args arguments to pass to the object's constructor.
        @return pointer to garbage-collected object.
        @exception std::bad_alloc thrown if there is not enough memory.
     */
    template <class T, class... Args> Ptr<T> make(Args&&... args) {
        static ObjectManager<T> om;

        //while construction, there shall not be a collection,
        //because during construction the pointer we have, i.e. 'this',
        //is not relocatable
        MakePrivate::Lock lock;

        //allocate memory
        void* mem = MakePrivate::allocate(sizeof(T), &om);

        //construct the object
        try {
            new (mem) T(std::forward<Args>(args)...);
        }
        catch (...) {
            MakePrivate::markInvalid(mem);
            throw;
        }

        //now that the object has been successfully constructed,
        //set its status so as that the collector knows its valid
        MakePrivate::markValid(mem);

        return result;
    }


} //namespace gclib


#endif //GCLIB_MAKE_HPP
