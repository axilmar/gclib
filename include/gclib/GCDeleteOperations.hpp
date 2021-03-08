#ifndef GCLIB_GCDELETEOPERATIONS_HPP
#define GCLIB_GCDELETEOPERATIONS_HPP


template <class T> class GCPtr;


//internal class with delete algorithms.
class GCDeleteOperations {
private:
    //unregister an allocation
    static void unregisterAllocation(void* block);

    //delete operation
    static void gcdelete(void* ptr);

    //gcnew uses the 'unregisterAllocation' function
    template <class T, class Malloc, class Init, class VTable> friend GCPtr<T> gcnew(size_t size, Malloc&& malloc, Init&& init, VTable& vtable);

    //GCPtr class uses the function 'gcdelete'
    template <class T> friend class GCPtr;

    //global delete function uses the function 'gcdelete'
    template <class T> friend void gcdelete(GCPtr<T>&& ptr);


};


#endif //GCLIB_GCDELETEOPERATIONS_HPP
