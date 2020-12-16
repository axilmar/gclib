#include <mutex>
#include <shared_mutex>
#include <type_traits>


class NoMutex {
public:
    void lock() {
    }

    void unlock() {
    }
};


//using MutexType = std::mutex;
//using MutexType = std::recursive_mutex;
//using MutexType = std::shared_mutex;
using MutexType = NoMutex;


class VoidPtr {
public:
    VoidPtr(void* value = nullptr);

    VoidPtr(const VoidPtr& ptr);

    VoidPtr(VoidPtr&& ptr);

    ~VoidPtr();

    VoidPtr& operator = (void* value);

    VoidPtr& operator = (const VoidPtr& ptr);

    VoidPtr& operator = (VoidPtr&& ptr);

    void* get() const noexcept {
        return m_value;
    }

    operator void*() const noexcept {
        return m_value;
    }

private:
    VoidPtr* m_prev;
    VoidPtr* m_next;
    void* m_value;
    MutexType* m_mutex;
};


template <class T> class Ptr : private VoidPtr {
public:
    Ptr(T* value = nullptr) : VoidPtr(value) {
    }

    Ptr(const Ptr& ptr) : VoidPtr(ptr) {
    }

    Ptr(Ptr&& ptr) : VoidPtr(std::move(ptr)) {
    }

    template <class U, typename = std::enable_if_t<std::is_base_of<T, U>, int>>
    Ptr(const Ptr<U>& ptr) : VoidPtr(ptr) {
    }

    template <class U, typename = std::enable_if_t<std::is_base_of<T, U>, int>>
    Ptr(Ptr<U>&& ptr) : VoidPtr(std::move(ptr)) {
    }

    Ptr& operator = (T* value) {
        VoidPtr::operator = (value);
        return *this;
    }

    Ptr& operator = (const Ptr& ptr) {
        VoidPtr::operator = (ptr);
        return *this;
    }

    Ptr& operator = (Ptr&& ptr) {
        VoidPtr::operator = (std::move(ptr));
        return *this;
    }

    template <class U, typename = std::enable_if_t<std::is_base_of<T, U>, int>>
    Ptr& operator = (const Ptr<U>& ptr) {
        VoidPtr::operator = (ptr);
        return *this;
    }

    template <class U, typename = std::enable_if_t<std::is_base_of<T, U>, int>>
    Ptr& operator = (Ptr<U>&& ptr) {
        VoidPtr::operator = (std::move(ptr));
        return *this;
    }

    T* get() const noexcept {
        return (T*)VoidPtr::get();
    }

    operator T*() const noexcept {
        return get();
    }

    T& operator *() const noexcept {
        return *get();
    }

    T* operator ->() const noexcept {
        return get();
    }
};


class GC {
public:
    static void init(std::size_t blockCount, std::size_t memSize);

private:
    static void* beginMalloc(std::size_t size, void (*finalizer)(void*, void*), void*& t) noexcept;
    static void endMalloc(void* t) noexcept;
    static void free(void* mem, void* t) noexcept;
    template <class T, class... Args> friend Ptr<T> gc_new(Args&&...);
};


template <class T> struct Finalizer {
    static void finalize(void* start, void* end) {
        ((T*)start)->~T();
    }
};


template <class T, class... Args> Ptr<T> gc_new(Args&&... args) {
    void* t;
    void* mem = GC::beginMalloc(sizeof(T), &Finalizer<T>::finalize, t);
    try {
        new (mem) T(std::forward<Args>(args)...);
    }
    catch (...) {
        GC::free(mem, t);
        throw;
    }
    GC::endMalloc(t);
    return (T*)mem;
}


#include <atomic>


template <class T> struct DNode {
    T* m_prev;
    T* m_next;

    void detach()  noexcept {
        ((DNode<T>*)m_prev)->m_next = m_next;
        ((DNode<T>*)m_next)->m_prev = m_prev;
    }
};


template <class T> struct DList : DNode<T> {
    using DNode<T>::m_prev;
    using DNode<T>::m_next;

    DList() noexcept {
        m_prev = m_next = (T*)this;
    }

    void append(T* obj) noexcept {
        ((DNode<T>*)obj)->m_prev = m_prev;
        ((DNode<T>*)obj)->m_next = (T*)this;
        ((DNode<T>*)m_prev)->m_next = obj;
        m_prev = obj;
    }
};


using PtrList = DList<VoidPtr>;


struct Block {
    PtrList ptrs;
    void(*finalizer)(void*, void*);
};


struct ThreadData {
    MutexType mutex;
};


struct Thread {
    ThreadData* data{ new ThreadData };
    MutexType rootMutex;
    PtrList rootPtrs;
    MutexType* mutex{ &rootMutex };
    DList<VoidPtr>* ptrs{&rootPtrs};
};


static thread_local Thread thread;
static std::atomic<Block**> blockPtrMem{ nullptr };
static std::atomic<Block**> blockPtrMemTop{ nullptr };
static Block** blockPtrMemTopEnd{ nullptr };
static std::atomic<char*> mem{ nullptr };
static char* memEnd{ nullptr };
static std::atomic<char*> memTop{ nullptr };


static Block* allocateMemory(std::size_t size) {
    Block* res = (Block*)memTop.fetch_add(size, std::memory_order_acq_rel);
    if ((char*)res + size <= memEnd) {
        Block** block = blockPtrMemTop.fetch_add(1, std::memory_order_acq_rel);
        if (block < blockPtrMemTopEnd) {
            *block = res;
            return res;
        }
        else {
            //TODO
            throw 1;
        }
    }

    //TODO
    throw 0;
}


VoidPtr::VoidPtr(void* value) : m_value(value) {
    m_mutex = thread.mutex;
    m_mutex->lock();
    thread.ptrs->append(this);
    m_mutex->unlock();
}


VoidPtr::VoidPtr(const VoidPtr& ptr) {
    m_mutex = thread.mutex;
    m_mutex->lock();
    m_value = ptr.m_value;
    thread.ptrs->append(this);
    m_mutex->unlock();
}


VoidPtr::VoidPtr(VoidPtr&& ptr) {
    m_mutex = thread.mutex;
    m_mutex->lock();
    m_value = ptr.m_value;
    ptr.m_value = nullptr;
    thread.ptrs->append(this);
    m_mutex->unlock();
}


VoidPtr::~VoidPtr() {
    m_mutex->lock();
    ((DNode<VoidPtr>*)this)->detach();
    m_mutex->unlock();
}


VoidPtr& VoidPtr::operator = (void* value) {
    thread.rootMutex.lock();
    m_value = value;
    thread.rootMutex.unlock();
    return *this;
}


VoidPtr& VoidPtr::operator = (const VoidPtr& ptr) {
    thread.rootMutex.lock();
    m_value = ptr.m_value;
    thread.rootMutex.unlock();
    return *this;
}


VoidPtr& VoidPtr::operator = (VoidPtr&& ptr) {
    thread.rootMutex.lock();
    void* const temp = ptr.m_value;
    ptr.m_value = nullptr;
    m_value = temp;
    thread.rootMutex.unlock();
    return *this;
}


void* GC::beginMalloc(std::size_t size, void(*finalizer)(void*, void*), void*& t) noexcept {
    size += sizeof(Block);
    Block* block = allocateMemory(size);
    if (!block) return nullptr;
    new (block) Block;
    block->finalizer = finalizer;
    t = thread.ptrs;
    thread.ptrs = &block->ptrs;
    return block + 1;
}


void GC::endMalloc(void* t) noexcept {
    thread.ptrs = (PtrList*)t;
}


void GC::free(void* mem, void* t) noexcept {
    thread.ptrs = (PtrList*)t;
}


void GC::init(std::size_t blockCount, std::size_t memSize) {
    blockPtrMem = blockPtrMemTop = (Block**)::operator new(blockCount * sizeof(Block*));
    blockPtrMemTopEnd = blockPtrMem + blockCount;
    mem = memTop = (char*)::operator new(memSize);
    memEnd = mem.load() + memSize;
}


#include <iostream>
#include <chrono>


template <class F> double time_function(F&& func) {
    auto start = std::chrono::high_resolution_clock::now();
    func();
    auto end = std::chrono::high_resolution_clock::now();
    return std::chrono::duration_cast<std::chrono::duration<double>>(end - start).count();
}


struct Foo
{
    Ptr<Foo> left;
    Ptr<Foo> right;
};


Ptr<Foo> func(int depth) {
    Ptr<Foo> foo = gc_new<Foo>();
    if (depth > 1)
    {
        foo->left = func(depth - 1);
        foo->right = func(depth - 1);
    }
    return foo;
}


int main() {
    static constexpr std::size_t MAX_DEPTH = 24;

    GC::init((1 << MAX_DEPTH), (sizeof(Block) + sizeof(Foo)) * ((1 << MAX_DEPTH)));


    auto dur = time_function([]() {
        Ptr root = func(MAX_DEPTH);
    });
    
    std::cout << dur << std::endl;
    system("pause");
    return 0;
}
