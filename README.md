# gclib

gclib is a C++17 garbage-collection library with the following features:

- 100% c++17 portable code.
- precise collection; only actual pointers are traced.
- implements the mark & sweep algorithm.
- can be used concurrently by multiple threads; very low (almost non-existent) lock contention.
- allows pointers to the middle of objects or object arrays.

## Classes

- GCPtr< T > : 'fat' smart pointer class that is automatically traced.
- GCBasicPtr< T > : lighter version of the above that is manually traced.
- GCBlockHeaderVTable< T > : allows full customization of memory management for the given type.
- GCIScannableObject : provides the interface for classes with manually traced pointers.
- GC : provides the garbage-collection functionality.

## Functions

- gcnew< T > : allocates a garbage-collected object.
- gcdelete< T > : deallocates a garbage-collected object.
- gcnewArray< T > : allocates a garbage-collected object array.
- gcdeleteArray< T > : deallocates a garbage-collected object array.

## Example

```cpp
#include "gclib.hpp"

//a class that uses garbage collection
class MyClass {
public:
	GCPtr<MyClass> other;
};

int main() {
	//create two garbage collected objects
	GCPtr<MyClass> object1{gcnew<MyClass>()};
	GCPtr<MyClass> object2{gcnew<MyClass>()};

	//form a cycle
	object1->other = object2;
	object2->other = object1;

	//remove root references
	object1 = nullptr;
	object2 = nullptr;

	//collect garbage; the above objects will be collected
	GC::collect();

	return 0;
}
```

## More Examples

The file `main.cpp` contains tests/examples for all features of this library.
