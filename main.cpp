#include <type_traits>
#include "tmp.h"

// Macros used to create a NOISE macro that forces __COUNTER__ to increase
#define _WRAP_PASTE(x,y) x##y
#define _MERGE(x,y) _WRAP_PASTE(x,y)
#define NOISE auto _MERGE(unused, __COUNTER__) = __COUNTER__

/*
   Class hierarchies
                                    F
                                   / \
     A                            H   \
    / \                          / \   \
   B   C                        I   J   G
  /   / \                        \ /   / \
 T   D   E                        K   L   Z         */
class A { };                   class F { };
class B : public A { };        class G : public F { };
class C : public A { };        class L : public G { };
class T : public B { };        class Z : public G { };
class D : public C { };        class H : public F { };
class E : public C { };        class I : public H { };
                               class J : public H { };
                               class K : public I, public J { };


// Declare the meta-variable
DECLARE_TL(registry);

// Add new types and stress test the solution with calls to __COUNTER__
ADD_TL(registry, I); NOISE;
ADD_TL(registry, C);
ADD_TL(registry, Z); NOISE; NOISE;
ADD_TL(registry, G); NOISE; NOISE; NOISE;
ADD_TL(registry, D);
ADD_TL(registry, F);
ADD_TL(registry, L); NOISE;
ADD_TL(registry, C); NOISE; NOISE; NOISE; NOISE; 
ADD_TL(registry, I); NOISE;
ADD_TL(registry, A); NOISE;
ADD_TL(registry, T);
ADD_TL(registry, B);
ADD_TL(registry, J);
ADD_TL(registry, K); NOISE; NOISE; NOISE; NOISE; NOISE;
ADD_TL(registry, H);
ADD_TL(registry, E);

int main()
{
    // Check that the constructed type list is what we expected
    using expected_registry = tmp::typelist<I, C, Z, G, D, F, L, C, I, A, T, B, J, K, H, E>;
    static_assert(
        std::is_same<READ_TL(registry), expected_registry>::value,
        "Error: unexpected registry type list"
    );

    return 0;
} 
