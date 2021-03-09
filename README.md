# gclib

gclib is a C++17 garbage-collection library with the following features:

- 100% c++17 portable code.
- precise collection; only actual pointers are traced.
- implements the mark & sweep algorithm.
- can be used concurrently by multiple threads; very low (almost non-existent) lock contention.
- allows pointers to the middle of objects or object arrays.
- allows garbage-collected objects to be allocated statically, i.e. as global/local/member variables.
- full integration with shared pointers.

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

The file `tests/main.cpp` contains tests/examples for all features of this library.

## Integration with shared pointers.

An object allocated with `gcnew` can also be managed via std::shared_ptr. The following principles apply:

- the object must inherit from std::enable_shared_from_this, in order to make the GC recognize the object is being shared; this is statically enforced anyway: a compilation error will be issued if this precondition is not met.
- an object that is to be deleted by the collector and it is shared via shared ptrs is not actually deleted by the collector; it will be deleted via its last shared ptr.
- an object that is to be deleted via its last shared ptr and not having been recognized as being unreachable by the collector will not be deleted via its last shared ptr; it will be deleted by the collector.
- the above decisions ensure that when an object is deleted, there are no gc or shared ptrs left dangling.
