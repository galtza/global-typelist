#include <iostream>
#include <iomanip>
#include <array>
#include <utility>
#include <stdio.h>
#include "tmp.h"

using namespace tmp;

template<typename TL>
struct hierarchy_iterator {
    static_assert(is_typelist<TL>::value, "Not a typelist");
    inline static void exec(void* _p) {
        using target_t = typename pop_front<TL>::type;
        if (auto ptr = static_cast<target_t*>(_p)) {
            printf("base = %s\n", typeid(typename at<TL, 0>::type).name());
            hierarchy_iterator<target_t>::exec(_p);
        }
    }
};

template<>
struct hierarchy_iterator<typelist<>> {
    inline static void exec(void*) {
    }
};

/*
=================================================================
                                    F
                                   / \
     A                            H   \
    / \                          / \   \
   B   C                        I   J   G
  /   / \                        \ /   / \
 T   D   E                        K   L   Z
================================================================= */

class A { };                   class F { };
class B : public A { };        class G : public F { };
class C : public A { };        class L : public G { };
class T : public B { };        class Z : public G { };
class D : public C { };        class H : public F { };
class E : public C { };        class I : public H { };
                               class J : public H { };
                               class K : public I, public J { };

// ==============================================================
#define _T(x) #x
#define STR(x) _T(x)
#define CONCAT(x, y) (x ## y)
#define MSG "ERROR at line " STR(__LINE__)
// ==============================================================
#define _WRAP_PASTE(x,y) x##y
#define _MERGE(x,y) _WRAP_PASTE(x,y)
#define _UNIQUE_UNUSED _MERGE(unused, __COUNTER__)
// ==============================================================

#define SERIALIZE(_instance, _tl) hierarchy_iterator<find_ancestors<_tl, decltype(_instance)>::type>::exec(&_instance)
#define NOISE auto _UNIQUE_UNUSED = __COUNTER__

DECLARE_TL(registry);

ADD_TL(registry, I);

NOISE;
ADD_TL(registry, C);
ADD_TL(registry, Z);
NOISE;
NOISE;
ADD_TL(registry, G);
NOISE;
NOISE;
ADD_TL(registry, D);
ADD_TL(registry, F);
ADD_TL(registry, L);
NOISE;
NOISE;
ADD_TL(registry, C);
NOISE;
ADD_TL(registry, I);
NOISE;
ADD_TL(registry, A);
NOISE;
ADD_TL(registry, T);
ADD_TL(registry, B);
ADD_TL(registry, J);
ADD_TL(registry, K);
NOISE;
NOISE;
NOISE;
NOISE;
NOISE;
ADD_TL(registry, H);
ADD_TL(registry, E);
ADD_TL(registry, E);

int main()
{
    //using namespace std;
    //using namespace tmp;

    //static_assert(is_same<READ_TL(registry),                          typelist<I, C, Z, G, D, F, L, C, I, A, T, B, J, K, H, E, E>>::value, MSG);
    //static_assert(is_same<find_ancestors<READ_TL(registry), D>::type, typelist<A, C, D>>::value,                                           MSG);
    //static_assert(is_same<find_ancestors<READ_TL(registry), K>::type, typelist<F, H, J, I, K>>::value,                                     MSG);

    //D d_instance;
    //SERIALIZE(d_instance, READ_TL(registry));

    //printf("\n\n");

    //K k_instance;
    //SERIALIZE(k_instance, READ_TL(registry));

    return 0;
}

