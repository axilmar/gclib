#include <mutex>
#include <shared_mutex>
#include <atomic>
#include <type_traits>


class NoMutex {
public:
    void lock() {
    }

    void unlock() {
    }

    void unlockNotify() {
    }
};


class Mutex {
public:
    bool try_lock();

    void lock();

    void unlock();

    void lockForGC();

    void unlockForGC();

private:
    std::atomic<bool> m_flag{ false };
    std::condition_variable m_cond;
};


//using MutexType = std::mutex;
//using MutexType = std::recursive_mutex;
//using MutexType = std::shared_mutex;
//using MutexType = NoMutex;
using MutexType = Mutex;


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

    void attach();
};


template <class T> class Ptr : private VoidPtr {
public:
    Ptr(T* value = nullptr) : VoidPtr(value) {
    }

    Ptr(const Ptr& ptr) : VoidPtr(ptr) {
    }

    Ptr(Ptr&& ptr) : VoidPtr(std::move(ptr)) {
    }

    template <class U, typename = std::enable_if_t<std::is_base_of_v<T, U>, int>>
    Ptr(const Ptr<U>& ptr) : VoidPtr(ptr) {
    }

    template <class U, typename = std::enable_if_t<std::is_base_of_v<T, U>, int>>
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

    template <class U, typename = std::enable_if_t<std::is_base_of_v<T, U>, int>>
    Ptr& operator = (const Ptr<U>& ptr) {
        VoidPtr::operator = (ptr);
        return *this;
    }

    template <class U, typename = std::enable_if_t<std::is_base_of_v<T, U>, int>>
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
    static void init(std::size_t memSize);

private:
    static void* beginMalloc(std::size_t size, void (*finalizer)(void*, void*), void*& t);
    static void endMalloc(void* t) noexcept;
    static void free(void* gcMem, void* t) noexcept;
    template <class T, class... Args> friend Ptr<T> gcnew(Args&&...);
};


template <class T> struct Finalizer {
    static void finalize(void* start, void* end) {
        ((T*)start)->~T();
    }
};


template <class T, class... Args> Ptr<T> gcnew(Args&&... args) {
    void* t;
    void* gcMem = GC::beginMalloc(sizeof(T), &Finalizer<T>::finalize, t);
    try {
        Ptr<T> ptr = (T*)gcMem;
        new (gcMem) T(std::forward<Args>(args)...);
        GC::endMalloc(t);
        return ptr;
    }
    catch (...) {
        GC::free(gcMem, t);
        throw;
    }
    return nullptr;
}


void gcdelete(void* gcMem) noexcept;

void collectGarbage();


#include <atomic>
#include <algorithm>


#define UNMARKED ((char*)0)
#define MARKED   ((char*)1)


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

    bool empty() const noexcept {
        return m_prev == m_next;
    }

    T* first() const noexcept {
        return m_next;
    }

    T* last() const noexcept {
        return m_prev;
    }

    const void* end() const noexcept {
        return this;
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
    char* newAddr;
    char* end;
    void(*finalizer)(void*, void*);
};


struct PTR {
    VoidPtr* m_prev;
    VoidPtr* m_next;
    char* m_value;
    void* m_mutex;
};


struct ThreadData : DNode<ThreadData>
{
    MutexType mutex;
    PtrList rootPtrs;
    bool empty() const noexcept { return rootPtrs.empty(); }
};


struct Thread : DNode<Thread> {
    ThreadData* data{ new ThreadData };
    MutexType& mutex{ data->mutex };
    PtrList& rootPtrs{ data->rootPtrs };
    DList<VoidPtr>* ptrs{ &data->rootPtrs };
    Thread();
    ~Thread();
};


class BadAlloc : public std::bad_alloc {
public:
    BadAlloc() {
    }

    const char* what() const noexcept final {
        return "GC out of memory";
    }
};


static thread_local Thread gcThread;
static char* gcMem{ nullptr };
static char* gcMemEnd{ nullptr };
static std::atomic<char*> gcMemTop{ nullptr };
static std::atomic<char*> gcMemTopGood{ nullptr };
static std::atomic<bool> gcFlag{false};
static std::mutex gcMutex;
static std::condition_variable gcWaitCollectionCond;
static DList<Thread> gcThreads;
static DList<ThreadData> gcThreadData;
static Mutex gcPtrMutex;
static PtrList gcPtrs;
static std::vector<Block*> gcAllBlocks;
static std::vector<Block*> gcMarkedBlocks;
static std::vector<std::pair<char**, Block*>> gcDirtyPtrs;


Thread::Thread() {
    gcMutex.lock();
    gcThreads.append(this);
    gcMutex.unlock();
}


Thread::~Thread() {
    gcMutex.lock();
    detach();
    if (data->empty()) {
        gcMutex.unlock();
        delete data;
    }
    else {
        gcThreadData.append(data);
        gcMutex.unlock();
    }
}


static void threadWaitGC(std::condition_variable& cond) {
    std::mutex mutex;
    std::unique_lock lock(mutex);
    gcWaitCollectionCond.wait(lock);
}


static char* allocateMemoryHelper(std::size_t size) {
    char* res = gcMemTop.fetch_add(size, std::memory_order_acq_rel);
    char* end = res + size;

    if (end <= gcMemEnd) {
        gcMemTopGood.store(res, std::memory_order_release);
        if (gcFlag.load(std::memory_order_acquire)) {
            threadWaitGC(gcWaitCollectionCond);
        }
        return res;
    }

    gcMemTop.fetch_sub(size, std::memory_order_acq_rel);
    return nullptr;
}


static char* allocateMemory(std::size_t size) {
    char* res = allocateMemoryHelper(size);
    if (res) return res;
    collectGarbage();
    res = allocateMemoryHelper(size);
    if (res) return res;
    throw BadAlloc();
}


static bool stopThreads() {   
    bool prev = false;
    if (!gcFlag.compare_exchange_strong(prev, true, std::memory_order_acq_rel)) {
        threadWaitGC(gcWaitCollectionCond);
        return false;
    }

    gcMutex.lock();

    for (Thread* thread = gcThreads.first(); thread != gcThreads.end(); thread = thread->m_next) {
        if (thread == &gcThread) continue;
        thread->mutex.lockForGC();
    }

    for (ThreadData* threadData = gcThreadData.first(); threadData != gcThreadData.end(); threadData = threadData->m_next) {
        threadData->mutex.lockForGC();
    }

    return true;
}


static void resumeThreads() noexcept {
    gcFlag.store(false, std::memory_order_release);
    
    for (ThreadData* threadData = gcThreadData.last(); threadData != gcThreadData.end(); threadData = threadData->m_prev) {
        threadData->mutex.unlockForGC();
    }
    
    for (Thread* thread = gcThreads.last(); thread != gcThreads.end(); thread = thread->m_prev) {
        if (thread == &gcThread) continue;
        thread->mutex.unlockForGC();
    }
    
    gcMutex.unlock();
    gcWaitCollectionCond.notify_all();
}


static void getAllBlocks() noexcept {
    char* memTop = gcMemTopGood.load(std::memory_order_acquire);
    for (Block* block = (Block*)gcMem; block < (Block*)memTop; block = (Block*)block->end) {
        gcAllBlocks.push_back(block);
    }
}


static Block* findBlock(void* addr) noexcept {
    return *--std::upper_bound(gcAllBlocks.begin(), gcAllBlocks.end(), addr, [](void* addr, Block* block) {
        return addr < block;
    });
}


static void scan(PTR* ptr);


template <class T> static void addDirtyPtr(T*& ptr, Block* block) {
    gcDirtyPtrs.emplace_back((char**)&ptr, block);
}


static void mark(Block* block) noexcept {
    if (block->newAddr == MARKED) return;
    block->newAddr = MARKED;
    gcMarkedBlocks.push_back(block);
    addDirtyPtr(block->ptrs.m_prev, block);
    addDirtyPtr(block->ptrs.m_next, block);
    for (PTR* ptr = (PTR*)block->ptrs.first(); ptr != block->ptrs.end(); ptr = (PTR*)((PTR*)ptr)->m_next) {
        addDirtyPtr(ptr->m_prev, block);
        addDirtyPtr(ptr->m_next, block);
        scan(ptr);
    }
}


static void scan(PTR* ptr) {
    if (ptr->m_value >= gcMem && ptr->m_value < gcMemEnd) {
        Block* const block = findBlock(ptr->m_value);
        mark(block);
        addDirtyPtr(ptr->m_value, block);
    }

}


static void scan(const PtrList& ptrs) noexcept {
    for (PTR* ptr = (PTR*)ptrs.first(); ptr != ptrs.end(); ptr = (PTR*)((PTR*)ptr)->m_next) {
        scan(ptr);
    }
}


static void mark() noexcept {
    getAllBlocks();

    for (Thread* thread = gcThreads.first(); thread != gcThreads.end(); thread = thread->m_next) {
        scan(thread->rootPtrs);
    }
    
    for (ThreadData* threadData = gcThreadData.first(); threadData != gcThreadData.end(); threadData = threadData->m_next) {
        scan(threadData->rootPtrs);
    }
}


static void finalize(Block* block) {
    block->finalizer(block + 1, block->end);
}


static void computeNewAddresses() {
    std::sort(gcMarkedBlocks.begin(), gcMarkedBlocks.end());
    char *newMemTop = gcMem;    
    for (Block* block : gcMarkedBlocks) {
        block->newAddr = newMemTop;
        const std::size_t blockSize = block->end - (char*)block;
        newMemTop += blockSize;
    }
    gcMemTop.store(newMemTop, std::memory_order_release);
}


static void adjustPtr(char*& ptr, Block* block) {
    char* const oldPtrValue = (char*)ptr;
    const std::size_t offset = oldPtrValue - (char*)block;
    char* const newPtrValue = block->newAddr + offset;
    (char*&)ptr = newPtrValue;
}


static void adjustPtrs() {
    for (const auto& p : gcDirtyPtrs) {
        adjustPtr(*p.first, (Block*)p.second);
    }
}


static void clearMemory() {
    for (Block* block : gcAllBlocks) {
        if (block->newAddr == UNMARKED) {
            finalize(block);
        }
        else {
            const std::size_t blockSize = block->end - (char*)block;
            block = (Block*)memmove(block->newAddr, block, blockSize);
            block->newAddr = UNMARKED;
            block->end = (char*)block + blockSize;
        }
    }
    gcAllBlocks.clear();
    gcMarkedBlocks.clear();
    gcDirtyPtrs.clear();
}


static std::vector<ThreadData*> sweepDeadThreads() {
    std::vector<ThreadData*> threadDataToDelete;
    for (ThreadData* data = gcThreadData.first(); data != gcThreadData.end();) {
        if (data->empty()) {
            ThreadData* next = data->m_next;
            data->detach();
            threadDataToDelete.push_back(data);
            data = next;
        }
        else {
            data = data->m_next;
        }
    }
    return threadDataToDelete;
}


static std::vector<ThreadData*> sweep() noexcept {
    computeNewAddresses();
    adjustPtrs();
    clearMemory();
    return sweepDeadThreads();
}


bool Mutex::try_lock() {
    bool prev = false;
    return m_flag.compare_exchange_strong(prev, true, std::memory_order_acquire, std::memory_order_relaxed);
}


void Mutex::lock() {
    bool prev = false;
    while (!m_flag.compare_exchange_weak(prev, true, std::memory_order_acquire, std::memory_order_relaxed)) {
        if (!gcFlag.load(std::memory_order_acquire)) {
            std::this_thread::yield();
        }
        else {
            threadWaitGC(m_cond);
        }
    }
}


void Mutex::unlock() {
    m_flag.store(false, std::memory_order_release);
}


void Mutex::lockForGC() {
    bool prev = false;
    while (!m_flag.compare_exchange_weak(prev, true, std::memory_order_acquire, std::memory_order_relaxed)) {
        std::this_thread::yield();
    }
}


void Mutex::unlockForGC() {
    unlock();
    m_cond.notify_all();
}


VoidPtr::VoidPtr(void* value) : m_value(value) {
    gcThread.mutex.lock();
    attach();
    gcThread.mutex.unlock();
}


VoidPtr::VoidPtr(const VoidPtr& ptr) {
    gcThread.mutex.lock();
    m_value = ptr.m_value;
    attach();
    gcThread.mutex.unlock();
}


VoidPtr::VoidPtr(VoidPtr&& ptr) {
    gcThread.mutex.lock();
    m_value = ptr.m_value;
    ptr.m_value = nullptr;
    attach();
    gcThread.mutex.unlock();
}


VoidPtr::~VoidPtr() {
    if (!m_mutex) return;
    m_mutex->lock();
    ((DNode<VoidPtr>*)this)->detach();
    m_mutex->unlock();
}


VoidPtr& VoidPtr::operator = (void* value) {
    gcThread.mutex.lock();
    m_value = value;
    gcThread.mutex.unlock();
    return *this;
}


VoidPtr& VoidPtr::operator = (const VoidPtr& ptr) {
    gcThread.mutex.lock();
    m_value = ptr.m_value;
    gcThread.mutex.unlock();
    return *this;
}


VoidPtr& VoidPtr::operator = (VoidPtr&& ptr) {
    gcThread.mutex.lock();
    void* const temp = ptr.m_value;
    ptr.m_value = nullptr;
    m_value = temp;
    gcThread.mutex.unlock();
    return *this;
}


void VoidPtr::attach() {
    if ((void*)this >= gcMem && (void*)this < gcMemEnd) {
        m_mutex = nullptr;
        gcThread.ptrs->append(this);
        return;
    }
    m_mutex = &gcThread.mutex;
    gcThread.rootPtrs.append(this);
}


void* GC::beginMalloc(std::size_t size, void(*finalizer)(void*, void*), void*& t) {
    size = (sizeof(Block) + size + sizeof(void*) - 1) & ~(sizeof(void*) - 1);
    Block* block = (Block*)allocateMemory(size);
    new (block) Block;
    block->newAddr = UNMARKED;
    block->end = (char*)block + size;
    block->finalizer = finalizer;
    t = gcThread.ptrs;
    gcThread.ptrs = &block->ptrs;
    return block + 1;
}


void GC::endMalloc(void* t) noexcept {
    gcThread.ptrs = (PtrList*)t;
}


void GC::free(void* gcMem, void* t) noexcept {
    gcThread.ptrs = (PtrList*)t;
    gcdelete(gcMem);
}


void GC::init(std::size_t memSize) {
    gcMem = gcMemTop = (char*)::operator new(memSize);
    gcMemEnd = gcMem + memSize;
}


void gcdelete(void* gcMem) noexcept {
    ((Block*)gcMem)->finalizer = nullptr;
}


void collectGarbage() {
    if (!stopThreads()) return;
    mark();
    const std::vector<ThreadData*> threadDataToDelete = sweep();
    resumeThreads();
    for (ThreadData* data : threadDataToDelete) {
        delete data;
    }
}


#include <iostream>
#include <chrono>
#include <memory>


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
    Ptr<Foo> foo = gcnew<Foo>();
    if (depth > 1)
    {
        foo->left = func(depth - 1);
        foo->right = func(depth - 1);
    }
    return foo;
}


static void stressTest() {
    static constexpr std::size_t MAX_DEPTH = 24;
    GC::init((sizeof(Block) + sizeof(Foo)) * ((1 << MAX_DEPTH)));

    auto dur = time_function([]() {
        Ptr root = func(MAX_DEPTH);
    });
    
    std::cout << dur << std::endl;
}


static void test() {
    GC::init(65536);

    //Ptr<Foo> foo1 = gcnew<Foo>();
    gcnew<Foo>();
    Ptr<Foo> foo2 = gcnew<Foo>();
    collectGarbage();
}


static void test1() {
    GC::init(65536);

    double dur = time_function([]() {
        for (int i = 0; i < 10'000'000; ++i) {
            Ptr<Foo> foo1 = gcnew<Foo>();
            Ptr<Foo> foo2 = gcnew<Foo>();
            foo1->right = foo2;
            foo2->left = foo1;
        }
    });

    collectGarbage();
    
    std::cout << dur << std::endl;
}


static void test2() {
    double dur = time_function([]() {
        for (int i = 0; i < 10'000'000; ++i) {
            std::shared_ptr<Foo> foo1 = std::make_shared<Foo>();
            std::shared_ptr<Foo> foo2 = std::make_shared<Foo>();
        }
    });

    std::cout << dur << std::endl;
}


static void test3() {
    GC::init(65535);

    double dur = time_function([]() {

        std::thread thread{[]() {
            for (int i = 0; i < 1'000'000; ++i) {
                Ptr<Foo> foo1 = gcnew<Foo>();
            }
        }};

        thread.join();
    });

    collectGarbage();
    
    std::cout << dur << std::endl;
}


static void test4() {
    GC::init(65535);

    double dur = time_function([]() {

        std::vector<std::thread> threads(1);

        for (std::thread& thread : threads) {
            thread = std::thread{ []() {
                for (int i = 0; i < 1'000'000; ++i) {
                    Ptr<Foo> foo1 = gcnew<Foo>();
                }
            }};
        }

        for (std::thread& thread : threads) {
            thread.join();
        }
    });

    //collectGarbage();
    
    std::cout << dur << std::endl;
}


int main() {
    test4();
    system("pause");
    return 0;
}

/* avoid finalizer, provide member pointers, manually check member pointers etc */