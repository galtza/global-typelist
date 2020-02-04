/*
    MIT License

    Copyright (c) 2019-2020 Raúl Ramos

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in all
    copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
    SOFTWARE.
*/
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

auto main() -> int {

    // Check that the constructed type list is what we expected
    using expected_registry = tmp::typelist<I, C, Z, G, D, F, L, C, I, A, T, B, J, K, H, E>;
    static_assert(
        std::is_same<READ_TL(registry), expected_registry>::value,
        "Error: unexpected registry type list"
    );

    return 0;
}
