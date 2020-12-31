#ifndef GCLIB_GCTRAITS_HPP
#define GCLIB_GCTRAITS_HPP


/**
 * Tests if a class has a member operator new.
 * @param T type of class to check.
 */
template <class T> struct GCHasOperatorNew {
private:
    template <class C> static char test(decltype(&C::operator new));
    template <class C> static int test(...);

public:
    ///true if the class has a member operator new, false otherwise.
    static constexpr bool Value = sizeof(test<T>(0)) == sizeof(char);
};


/**
 * Tests if a class has a member operator delete.
 * @param T type of class to check.
 */
template <class T> struct GCHasOperatorDelete {
private:
    template <class C> static char test(decltype(&C::operator delete));
    template <class C> static int test(...);

public:
    ///true if the class has a member operator delete, false otherwise.
    static constexpr bool Value = sizeof(test<T>(0)) == sizeof(char);
};


/**
 * Tests if a class has a member operator new[].
 * @param T type of class to check.
 */
template <class T> struct GCHasOperatorNew<T[]> {
private:
    template <class C> static char test(decltype(&C::operator new[]));
    template <class C> static int test(...);

public:
    ///true if the class has a member operator new[], false otherwise.
    static constexpr bool Value = sizeof(test<T>(0)) == sizeof(char);
};


/**
 * Tests if a class has a member operator delete[].
 * @param T type of class to check.
 */
template <class T> struct GCHasOperatorDelete<T[]> {
private:
    template <class C> static char test(decltype(&C::operator delete[]));
    template <class C> static int test(...);

public:
    ///true if the class has a member operator delete[], false otherwise.
    static constexpr bool Value = sizeof(test<T>(0)) == sizeof(char);
};


#endif //GCLIB_GCTRAITS_HPP
