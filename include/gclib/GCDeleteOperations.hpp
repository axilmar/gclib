#ifndef GCLIB_GCDELETEOPERATIONS_HPP
#define GCLIB_GCDELETEOPERATIONS_HPP


template <class T> class GCPtr;


//internal class with delete algorithms.
class GCDeleteOperations {
private:
    //unregisters a block
    static void unregisterBlock(class GCBlockHeader* block);

    //delete a block without unregistering it
    static void deleteBlock(class GCBlockHeader* block);

    //delete and unregister a block
    static void deleteAndUnregisterBlock(class GCBlockHeader* block);

    //delete operator
    static void operatorDelete(void* ptr);

    //delete block only if has already been collected
    static void operatorDeleteIfCollected(void* ptr);

    //gcnew uses the 'unregisterAllocation' function
    template <class T, class Malloc, class Init, class VTable> friend GCPtr<T> gcnew(size_t size, Malloc&& malloc, Init&& init, VTable& vtable);

    //GCPtr class uses the function 'gcdelete'
    template <class T> friend class GCPtr;

    //global delete function uses the function 'gcdelete'
    template <class T> friend void gcdelete(GCPtr<T>&& ptr);

    //internal function
    friend static void sweep(class GCBlockHeader* block);
};


#endif //GCLIB_GCDELETEOPERATIONS_HPP
