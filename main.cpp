#include <iostream>
#include <iomanip>
#include <array>
#include <utility>
#include <stdio.h>

namespace tmp {

    // type list
    template<typename...>
    struct typelist {
    };

    template<typename T>
    struct is_typelist : std::false_type { };

    template<typename...TS>
    struct is_typelist<typelist<TS...>> : std::true_type { };

    // basic operations
    template<typename T, typename TL> struct push_back;
    template<typename T, typename TL> struct push_front;
    template<typename TL>             struct pop_front;
    template<typename TL, size_t I>   struct at;

    template<typename T, typename...TS>
    struct push_back<T, typelist<TS...>> {
        using type = typelist<TS..., T>;
    };

    template<typename T, typename...TS>
    struct push_front<T, typelist<TS...>> {
        using type = typelist<T, TS...>;
    };

    template<typename T, typename...TS>
    struct pop_front<typelist<T, TS...>> {
        using type = typelist<TS...>;
    };

    template <typename T, typename...TS>
    struct at<typelist<T, TS...>, 0> {
        using type = T;
    };

    template <typename T, typename...TS, size_t I>
    struct at<typelist<T, TS...>, I> {
        using type = typename at<typelist<TS...>, I - 1>::type;
    };

    // 'filter'
    template<typename TL, template<typename>class PRED>
    struct filter;

    template <template<typename>class PRED>
    struct filter<typelist<>, PRED> {
        using type = typelist<>;
    };

    template <typename T, typename...TS, template<typename>class PRED>
    struct filter<typelist<T, TS...>, PRED> {
        using remaining = typename filter<typelist<TS...>, PRED>::type;
        using type = typename std::conditional<
            PRED<T>::value,
            typename push_front<T, remaining>::type,
            remaining
        >::type;
    };

    // 'max' given a template binary predicate
    template<typename TL, template<typename,typename>class PRED>
    struct max;

    template <typename T, template<typename,typename>class PRED>
    struct max<typelist<T>, PRED> {
        using type = T;
    };

    template <typename ...TS, template<typename,typename>class PRED>
    struct max<typelist<TS...>, PRED> {
        using first = typename at<typelist<TS...>, 0>::type;
        using remaining_max = typename max<typename pop_front<typelist<TS...>>::type, PRED>::type;
        using type  = typename std::conditional<
            PRED<first, remaining_max>::value,
            first, remaining_max
        >::type;
    };

    // 'find_ancestors'
    namespace impl {

        template<typename SRCLIST, typename DESTLIST>
        struct find_ancestors {

            template<typename B>
            using negation = typename std::integral_constant<bool, !bool(B::value)>::type;

            template<typename T, typename U>
            using cmp = typename std::is_base_of<T, U>::type;
            using most_ancient = typename max<SRCLIST, cmp>::type;

            template<typename T>
            using not_most_ancient = typename negation<std::is_same<most_ancient, T>>::type;

            using type = typename find_ancestors<
                typename filter<SRCLIST, not_most_ancient>::type,
                typename push_back<most_ancient, DESTLIST>::type
            >::type;

        };

        template<typename DESTLIST>
        struct find_ancestors<typelist<>, DESTLIST> {
            using type = DESTLIST;
        };

    }

    template<typename TL, typename T>
    struct find_ancestors {
        static_assert(is_typelist<TL>::value, "The first parameter is not a typelist");

        template<typename U>
        using base_of_T = typename std::is_base_of<U, T>::type;
        using src_list = typename filter<TL, base_of_T>::type;
        using type = typename impl::find_ancestors<
            src_list,
            typelist<>
        >::type;
    };

}

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

/* Init the system: */
/* 1.- Grab the current __COUNTER__ and generate the initial temporary registry typelist*/
namespace tmp {
    template<typename> struct start;
    template<>         struct start<size_t> : std::integral_constant<size_t, __COUNTER__> { };
    template<size_t> struct building_typelist;
    template<> struct building_typelist<start<size_t>::value> {
        using type = tmp::typelist<>;
    };
}

namespace tmp {

    template <typename T, size_t = sizeof(T)>
    auto is_class_complete(T*)  -> std::true_type;
    constexpr auto is_class_complete(...) -> std::false_type;

    template <size_t I>
    using is_defined = decltype(is_class_complete(std::declval<building_typelist<I>*>()));

    template<size_t I, bool = std::is_same<std::true_type, is_defined<I>>::value>
    struct select_current_types;

    template<size_t I>
    struct select_current_types<I, true> {
        using type = typename building_typelist<I>::type;
    };

    template<size_t I>
    struct select_current_types<I, false> {
        using type = typename std::conditional< 
            (I > start<size_t>::value),     // Are there more specializations to check?
            typename select_current_types<I-1>::type, // yes
            tmp::typelist<>                 // no => failed => empty typelist
        >::type;
    };

}

#define _REGISTER(_class, _idx)\
    template<>\
    struct building_typelist<_idx> {\
        using previous = typename tmp::select_current_types<_idx-1>::type;\
        using type = typename tmp::push_back<_class, previous>::type;\
    }

#define REGISTER(_class) _REGISTER(_class, __COUNTER__)

#define CURRENT_TYPELIST typename select_current_types<__COUNTER__>::type

REGISTER(I);
REGISTER(E);
auto T0 = __COUNTER__;
REGISTER(C);
REGISTER(D);
REGISTER(B);
auto T1 = __COUNTER__;
REGISTER(F);
REGISTER(H);
REGISTER(G);
auto T2 = __COUNTER__;
auto T3 = __COUNTER__;
auto T4 = __COUNTER__;
auto T5 = __COUNTER__;
auto T6 = __COUNTER__;
auto T7 = __COUNTER__;
REGISTER(A);


int main()
{
    using namespace std;
    using namespace tmp;

    static_assert(is_same<CURRENT_TYPELIST,                          typelist<I,E,C,D,B,F,H,G,A>>::value, MSG);
    static_assert(is_same<find_ancestors<CURRENT_TYPELIST, D>::type, typelist<A, C, D>>::value,           MSG);

    D d_instance;
    hierarchy_iterator<find_ancestors<CURRENT_TYPELIST, D>::type>::exec(&d_instance);

    return 0;
}
