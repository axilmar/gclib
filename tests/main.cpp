#include <iostream>
#include <chrono>
#include <vector>


template <class F> double timeFunction(F&& func) {
    auto start = std::chrono::high_resolution_clock::now();
    func();
    auto end = std::chrono::high_resolution_clock::now();
    return std::chrono::duration_cast<std::chrono::duration<double>>(end - start).count();
}


const std::size_t MaxObject = 5'000'000;


class Object1 {
public:
    char data[64];
};


std::vector<Object1*> object1;


void testObject1() {
    object1.resize(MaxObject);

    double dur = timeFunction([]() {
        for (size_t i = 0; i < MaxObject; ++i) {
            object1[i] = new Object1;
        }
        for (size_t i = 0; i < MaxObject; ++i) {
            delete object1[i];
        }
    });

    std::cout << dur << " seconds\n";
}


#include "gclib/MemoryResource.hpp"


using namespace gclib;


class Object2 {
public:
    char data[64];
    void* operator new(std::size_t size);
    void operator delete(void* mem);
};


MemoryResource mr;
std::vector<Object2*> object2;


void* Object2::operator new(std::size_t size) {
    return mr.allocate(size);
}


void Object2::operator delete(void* mem) {
    mr.deallocate(mem);
}


void testObject2() {
    object2.resize(MaxObject);

    double dur = timeFunction([]() {
        for (size_t i = 0; i < MaxObject; ++i) {
            object2[i] = new Object2;
        }
        for (size_t i = 0; i < MaxObject; ++i) {
            delete object2[i];
        }
    });

    std::cout << dur << " seconds\n";
}


int main() {
    testObject2();
    system("pause");
    return 0;
}
