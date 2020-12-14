#include <iostream>
#include "gclib/StackPtr.hpp"
#include "gclib/GlobalPtr.hpp"


using namespace gclib;


class Foo
{
public:
};


int main()
{
    StackPtr<Foo> foo1 = new Foo;
    GlobalPtr<Foo> foo2 = foo1;
    return 0;
}
