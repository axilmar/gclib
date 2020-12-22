#include <mutex>
#include <shared_mutex>
#include <type_traits>


template <class T> struct GCNode {
    T* prev;
    T* next;
    
    void detach() noexcept {
        prev->next = next;
        next->prev = prev;
    }
};


template <class T> struct GCList : GCNode<T> {
    using GCNode<T>::prev;
    using GCNode<T>::next;

    GCList() noexcept {
        clear();
    }

    GCList(const GCList&) = delete;

    GCList(GCList&& list) noexcept {
        operator = (std::move(list));
    }

    bool empty() const noexcept {
        return next == end();
    }

    T* first() const noexcept {
        return next;
    }

    T* last() const noexcept {
        return prev;
    }

    const void* end() const noexcept {
        return static_cast<const T*>(static_cast<const GCNode<T>*>(this));
    }

    void append(T* const node) noexcept {
        node->prev = prev;
        node->next = static_cast<T*>(static_cast<GCNode<T>*>(this));
        prev->next = node;
        prev = node;
    }

    void append(GCList<T>&& list) noexcept {
        if (list.empty()) return;
        list.next->prev = prev;
        list.prev->next = static_cast<T*>(static_cast<GCNode<T>*>(this));
        prev->next = list.next;
        prev = list.prev;
        list.clear();
    }

    void clear() noexcept {
        prev = next = static_cast<T*>(static_cast<GCNode<T>*>(this));
    }

    GCList& operator = (const GCList&) = delete;

    GCList& operator = (GCList&& list) noexcept {
        if (list.empty()) {
            clear();
        }
        else {
            list.prev->next = static_cast<T*>(static_cast<GCNode<T>*>(this));
            list.next->prev = static_cast<T*>(static_cast<GCNode<T>*>(this));
            next = list.next;
            prev = list.prev;
            list.clear();
        }
        return *this;
    }
};


class GCObjectPtr;


class GCObject : private GCNode<GCObject> {
public:
    GCObject();

    GCObject(const GCObject& object);

    GCObject(GCObject&& object);

    virtual ~GCObject();

    GCObject& operator = (const GCObject&) noexcept {
        return *this;
    }

    GCObject& operator = (GCObject&&) noexcept {
        return *this;
    }

private:
    struct ThreadData* m_threadData;
    GCList<GCObjectPtr> m_ptrs;
    const std::size_t m_size;
    std::size_t m_cycle{0};

    friend class GCObjectPtr;
    friend struct GCNode<GCObject>;
    friend struct GCList<GCObject>;
    friend struct GCPriv;
};


class GCObjectPtr : private GCNode<GCObjectPtr> {
public:
    typedef GCObject* ValueType;
    typedef GCObject& ReferenceType;

    GCObjectPtr(GCObject &owner, ValueType const value = nullptr);

    GCObjectPtr(GCObject &owner, const GCObjectPtr& ptr);

    GCObjectPtr(GCObject &owner, GCObjectPtr&& ptr);

    GCObjectPtr(ValueType const value = nullptr);

    GCObjectPtr(const GCObjectPtr& ptr);

    GCObjectPtr(GCObjectPtr&& ptr);

    ~GCObjectPtr();

    GCObjectPtr& operator = (ValueType const value);

    GCObjectPtr& operator = (const GCObjectPtr& ptr);

    GCObjectPtr& operator = (GCObjectPtr&& ptr);

    ValueType get() const noexcept {
        return m_value;
    }

    operator ValueType () const noexcept {
        return m_value;
    }

    ReferenceType operator *() const noexcept {
        return *m_value;
    }

    ValueType operator ->() const noexcept {
        return m_value;
    }

private:
    std::recursive_mutex* m_ownerMutex;
    ValueType m_value;
    
    friend struct GCNode<GCObjectPtr>;
    friend struct GCList<GCObjectPtr>;
    friend struct GCPriv;
};


template <class T> class GCPtr : public GCObjectPtr {
public:
    typedef T* ValueType;
    typedef T& ReferenceType;

    GCPtr(GCObject& owner, ValueType value = nullptr) 
        : GCObjectPtr(owner, value)
    {
    }

    GCPtr(GCObject& owner, const GCPtr& ptr) 
        : GCObjectPtr(owner, ptr)
    {
    }

    GCPtr(GCObject& owner, GCPtr&& ptr) 
        : GCObjectPtr(owner, std::move(ptr))
    {
    }

    template <class U, class = std::enable_if_t<std::is_base_of_v<T, U>, int>>
    GCPtr(GCObject& owner, const GCPtr<U>& ptr) 
        : GCObjectPtr(owner, ptr)
    {
    }

    template <class U, class = std::enable_if_t<std::is_base_of_v<T, U>, int>>
    GCPtr(GCObject& owner, GCPtr<U>&& ptr) 
        : GCObjectPtr(owner, std::move(ptr))
    {
    }

    GCPtr(ValueType value = nullptr) 
        : GCObjectPtr(value)
    {
    }

    GCPtr(const GCPtr& ptr) 
        : GCObjectPtr(ptr)
    {
    }

    GCPtr(GCPtr&& ptr) 
        : GCObjectPtr(std::move(ptr))
    {
    }

    template <class U, class = std::enable_if_t<std::is_base_of_v<T, U>, int>>
    GCPtr(const GCPtr<U>& ptr) 
        : GCObjectPtr(ptr)
    {
    }

    template <class U, class = std::enable_if_t<std::is_base_of_v<T, U>, int>>
    GCPtr(GCPtr<U>&& ptr) 
        : GCObjectPtr(std::move(ptr))
    {
    }

    GCPtr& operator = (ValueType value) {
        GCObjectPtr::operator = (value);
        return *this;
    }

    GCPtr& operator = (const GCPtr& ptr) {
        GCObjectPtr::operator = (ptr);
        return *this;
    }

    GCPtr& operator = (GCPtr&& ptr) {
        GCObjectPtr::operator = (std::move(ptr));
        return *this;
    }

    template <class U, class = std::enable_if_t<std::is_base_of_v<T, U>, int>>
    GCPtr& operator = (const GCPtr<U>& ptr) {
        GCObjectPtr::operator = (ptr);
        return *this;
    }

    template <class U, class = std::enable_if_t<std::is_base_of_v<T, U>, int>>
    GCPtr& operator = (GCPtr<U>&& ptr) {
        GCObjectPtr::operator = (std::move(ptr));
        return *this;
    }

    ValueType get() const noexcept {
        return static_cast<T*>(GCObjectPtr::get());
    }

    operator ValueType () const noexcept {
        return get();
    }

    ReferenceType operator *() const {
        return *get();
    }

    ValueType operator ->() const {
        return get();
    }
};


class GC {
public:
    static void collect();
};


class GCNewLock {
public:
    GCNewLock(const std::size_t size);
    GCNewLock(const GCNewLock&) = delete;
    GCNewLock(GCNewLock&&) = delete;
    ~GCNewLock();
};


template <class T, class... Args> GCPtr<T> gcnew(Args&&... args) {
    GCNewLock lock(sizeof(T));
    return new T(std::forward<Args>(args)...);
}


#include <atomic>


struct ThreadData : GCNode<ThreadData> {
    std::recursive_mutex mutex;
    GCList<GCObject> objects;
    GCList<GCObject> markedObjects;
    GCList<GCObjectPtr> ptrs;

    bool empty() const noexcept {
        return objects.empty() && ptrs.empty();
    }
};


static thread_local struct Thread {
    ThreadData* const data{ new ThreadData };
    std::size_t newObjectSize;
    Thread();
    ~Thread();
} thread;


static struct Global {
    std::mutex mutex;
    GCList<ThreadData> threadData;
    GCList<ThreadData> terminatedThreadData;
    std::size_t cycle{0};
    std::atomic<std::size_t> allocSize{0};
    std::atomic<std::size_t> maxAllocSize{64*1024*1024};
} global;


struct GCPriv {
    static GCObjectPtr* next(const GCObjectPtr* const ptr) noexcept {
        return ptr->next;
    }

    static std::size_t& cycle(GCObject* const obj) noexcept {
        return obj->m_cycle;
    }

    static const GCList<GCObjectPtr>& ptrs(const GCObject* const obj) noexcept {
        return obj->m_ptrs;
    }

    template <class T> static GCNode<T>* node(T* const obj) noexcept {
        return obj;
    }

    static ThreadData*& threadData(GCObject* const obj) noexcept {
        return obj->m_threadData;
    }

    static std::size_t size(const GCObject* const obj) noexcept {
        return obj->m_size;
    }

    static GCObject*& value(GCObjectPtr* const ptr) noexcept {
        return ptr->m_value;
    }

    static std::recursive_mutex*& ownerMutex(GCObjectPtr* const ptr) noexcept {
        return ptr->m_ownerMutex;
    }
};


Thread::Thread() {
    std::lock_guard lock(global.mutex);
    global.threadData.append(data);
}


Thread::~Thread() {
    std::lock_guard lock(global.mutex);
    data->detach();
    data->mutex.lock();
    const bool empty = data->empty();
    data->mutex.unlock();
    if (empty) {
        delete data;
    }
    else {
        global.terminatedThreadData.append(data);
    }
}


static void stopThreads() {
    global.mutex.lock();
    for (ThreadData* td = global.threadData.first(); td != global.threadData.end(); td = td->next) {
        td->mutex.lock();
    }
}


static void resumeThreads() {
    for (ThreadData* td = global.threadData.last(); td != global.threadData.end(); td = td->prev) {
        td->mutex.unlock();
    }
    global.mutex.unlock();
}


static void scan(const GCList<GCObjectPtr>& ptrs);


static void mark(GCObject* obj) {
    if (!obj || GCPriv::cycle(obj) == global.cycle) return;
    GCPriv::cycle(obj) = global.cycle;
    GCPriv::node(obj)->detach();
    GCPriv::threadData(obj)->markedObjects.append(obj);
    global.allocSize.fetch_add(GCPriv::size(obj), std::memory_order::memory_order_relaxed);
    scan(GCPriv::ptrs(obj));
}


static void scan(const GCList<GCObjectPtr>& ptrs) {
    for (const GCObjectPtr* ptr = ptrs.first(); ptr != ptrs.end(); ptr = GCPriv::next(ptr)) {
        mark(ptr->get());
    }
}


static void mark() {
    ++global.cycle;
    global.allocSize.store(0, std::memory_order_release);
    for (ThreadData* td = global.threadData.first(); td != global.threadData.end(); td = td->next) {
        scan(td->ptrs);
    }
}


static void sweep(GCList<GCObject>& unreachable, GCList<ThreadData>& dead) {
    for (ThreadData* td = global.threadData.first(); td != global.threadData.end(); td = td->next) {
        unreachable.append(std::move(td->objects));
        td->objects = std::move(td->markedObjects);
    }

    for (ThreadData* td = global.terminatedThreadData.first(); td != global.terminatedThreadData.end(); ) {
        if (td->empty()) {
            ThreadData* next = td->next;
            td->detach();
            dead.append(td);
            td = next;
        }
        else {
            unreachable.append(std::move(td->objects));
            td->objects = std::move(td->markedObjects);
            td = td->next;
        }
    }
}


static void resetPtrs(const GCList<GCObjectPtr>& ptrs) {
    for (GCObjectPtr* ptr = ptrs.first(); ptr != ptrs.end(); ptr = GCPriv::next(ptr)) {
        GCPriv::ownerMutex(ptr) = nullptr;
        GCPriv::value(ptr) = nullptr;
    }
}


static void dispose(GCObject* obj) {
    GCPriv::threadData(obj) = nullptr;
    resetPtrs(GCPriv::ptrs(obj));
    delete obj;
}


template <class T> static void dispose(T* obj) {
    delete obj;
}


template <class T> static void dispose(const GCList<T>& objects) {
    while (!objects.empty()) {
        T* obj = objects.first();
        GCPriv::node(obj)->detach();
        dispose(obj);
    }
}


static void countNewObject(const std::size_t size) {
    if (global.allocSize.fetch_add(size, std::memory_order_acquire) >= global.maxAllocSize.load(std::memory_order_acquire)) {
        GC::collect();
    }
    thread.newObjectSize = size;
}


GCObject::GCObject() 
    : m_threadData(thread.data)
    , m_size(thread.newObjectSize)
{
    thread.data->objects.append(this);
}


GCObject::GCObject(const GCObject& object)
    : m_threadData(thread.data)
    , m_size(thread.newObjectSize)
{
    thread.data->objects.append(this);
}


GCObject::GCObject(GCObject&& object)
    : m_threadData(thread.data)
    , m_size(thread.newObjectSize)
{
    thread.data->objects.append(this);
}


GCObject::~GCObject()
{
    if (!m_threadData) return;
    std::lock_guard lock(m_threadData->mutex);
    detach();
}


GCObjectPtr::GCObjectPtr(GCObject &owner, ValueType const value) 
    : m_ownerMutex(&owner.m_threadData->mutex), m_value(value)
{
    owner.m_ptrs.append(this);
}


GCObjectPtr::GCObjectPtr(GCObject &owner, const GCObjectPtr& ptr) 
    : m_ownerMutex(&owner.m_threadData->mutex), m_value(ptr.m_value)
{
    owner.m_ptrs.append(this);
}


GCObjectPtr::GCObjectPtr(GCObject &owner, GCObjectPtr&& ptr)
    : m_ownerMutex(&owner.m_threadData->mutex), m_value(ptr.m_value)
{
    owner.m_ptrs.append(this);
    ptr.m_value = nullptr;
}


GCObjectPtr::GCObjectPtr(ValueType const value) 
    : m_ownerMutex(&thread.data->mutex), m_value(value)
{
    std::lock_guard lock(*m_ownerMutex);
    thread.data->ptrs.append(this);
}


GCObjectPtr::GCObjectPtr(const GCObjectPtr& ptr) 
    : m_ownerMutex(&thread.data->mutex), m_value(ptr.m_value)
{
    std::lock_guard lock(*m_ownerMutex);
    thread.data->ptrs.append(this);
}


GCObjectPtr::GCObjectPtr(GCObjectPtr&& ptr)
    : m_ownerMutex(&thread.data->mutex), m_value(ptr.m_value)
{
    std::lock_guard lock(*m_ownerMutex);
    thread.data->ptrs.append(this);
    ptr.m_value = nullptr;
}


GCObjectPtr::~GCObjectPtr() {
    if (!m_ownerMutex) return;
    std::lock_guard lock(*m_ownerMutex);
    detach();
}


GCObjectPtr& GCObjectPtr::operator = (ValueType const value) {
    std::lock_guard lock(*m_ownerMutex);
    m_value = value;
    return *this;
}


GCObjectPtr& GCObjectPtr::operator = (const GCObjectPtr& ptr) {
    std::lock_guard lock(*m_ownerMutex);
    m_value = ptr.m_value;
    return *this;
}


GCObjectPtr& GCObjectPtr::operator = (GCObjectPtr&& ptr) {
    std::lock_guard lock(*m_ownerMutex);
    ValueType const temp = ptr.m_value;
    ptr.m_value = nullptr;
    m_value = temp;
    return *this;
}


void GC::collect() {
    stopThreads();
    mark();
    GCList<GCObject> unreachable;
    GCList<ThreadData> dead;
    sweep(unreachable, dead);
    resumeThreads();
    dispose(unreachable);
    dispose(dead);
}


GCNewLock::GCNewLock(const std::size_t size) {
    countNewObject(size);
    thread.data->mutex.lock();
}


GCNewLock::~GCNewLock() {
    thread.data->mutex.unlock();
}


#include <iostream>
#include <chrono>
#include <atomic>
#include <vector>
#include <thread>


template <class F> double timeFunction(F&& func) {
    auto start = std::chrono::high_resolution_clock::now();
    func();
    auto end = std::chrono::high_resolution_clock::now();
    return std::chrono::duration_cast<std::chrono::duration<double>>(end - start).count();
}


class Foo1 : public GCObject {
public:
    Foo1() {
    }
};


void test1() {
    const double dur = timeFunction([]() {
        for (std::size_t i = 0; i < 5'000'000; ++i) {
            GCObjectPtr foo1 = new Foo1();
        }
    });

    std::cout << dur << " seconds.\n";
}


static std::atomic<int> count{ 0 };


class Foo2 : public GCObject {
public:
    GCPtr<Foo2> other;

    Foo2() : other(*this) {
        ++count;
    }

    ~Foo2() {
        --count;
    }
};


void test2() {
    {
        GCPtr<Foo2> f1 = new Foo2();
        GCPtr<Foo2> f2 = new Foo2();
        f1->other = f2;
        f2->other = f1;
    }
    GC::collect();
    std::cout << "object count = " << count << std::endl;
}


void test3() {
    const std::size_t ThreadCount = 4;
    const std::size_t ObjectCount = 1'000'000;
    std::vector<std::thread> threads(ThreadCount);

    const double dur = timeFunction([&]() {
        for (std::thread& thread : threads) {
            thread = std::thread([&]() {
                for (std::size_t i = 0; i < ObjectCount; ++i) {
                    GCPtr<Foo2> foo{ gcnew<Foo2>() };
                }
            });
        }

        for (std::thread& thread : threads) {
            thread.join();
        }

        GC::collect();
    });

    std::cout << "duration = " << dur << " seconds" << std::endl;
    std::cout << "object count = " << count << std::endl;
}


int main() {
    test3();
    system("pause");
    return 0;
}
