#include <iostream>
#include <chrono>
#include "gclib/make.hpp"


template <class F> double timeFunction(F&& func) {
    auto start = std::chrono::high_resolution_clock::now();
    func();
    auto end = std::chrono::high_resolution_clock::now();
    return std::chrono::duration_cast<std::chrono::duration<double>>(end - start).count();
}


int main() {
    system("pause");
    return 0;
}
