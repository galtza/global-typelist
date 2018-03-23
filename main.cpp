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
#define _WRAP_PASTE(x,y) x##y
#define _MERGE(x,y) _WRAP_PASTE(x,y)
#define _UNIQUE_UNUSED _MERGE(unused, __COUNTER__)
// ==============================================================


namespace tmp {

    // Capture the current __COUNTER__ and create the "typelist_in_progress"
    template<typename> struct initial_counter;
    template<>         struct initial_counter<size_t> : std::integral_constant<size_t, __COUNTER__> { };
    template<size_t> struct typelist_in_progress;
    template<> struct typelist_in_progress<initial_counter<size_t>::value> {
        using type = tmp::typelist<>;
    };

    // Check if a class is defined or not
    template <typename T, size_t = sizeof(T)>
    auto is_class_complete(T*)  -> std::true_type;
    constexpr auto is_class_complete(...) -> std::false_type;
    template <size_t I>
    using is_defined = decltype(is_class_complete(std::declval<typelist_in_progress<I>*>()));

    // Select the current typelist at index "I"
    template<size_t I, bool = std::is_same<std::true_type, is_defined<I>>::value>
    struct select_types_at;

    template<size_t I>
    struct select_types_at<I, true> {
        using type = typename typelist_in_progress<I>::type;
    };

    template<size_t I>
    struct select_types_at<I, false> {
        using type = typename std::conditional< 
            (I > initial_counter<size_t>::value),     // Are there more specializations to check?
            typename select_types_at<I-1>::type, // yes
            tmp::typelist<>                 // no => failed => empty typelist
        >::type;
    };

}

#define _REGISTER(_class, _idx)\
    /* Define the current typelist at index _idx */\
    template<>\
    struct typelist_in_progress<_idx> {\
        using previous = typename tmp::select_types_at<_idx-1>::type;\
        using type = typename tmp::push_back<_class, previous>::type;\
    }

#define REGISTER(_class) _REGISTER(_class, __COUNTER__)
#define CURRENT_TYPELIST typename select_types_at<__COUNTER__>::type
#define SERIALIZE(_instance) hierarchy_iterator<find_ancestors<CURRENT_TYPELIST, decltype(_instance)>::type>::exec(&_instance)
#define NOISE auto _UNIQUE_UNUSED = __COUNTER__

REGISTER(I);
NOISE;
REGISTER(C);
REGISTER(Z);
NOISE;
NOISE;
REGISTER(G);
NOISE;
NOISE;
REGISTER(D);
REGISTER(F);
REGISTER(L);
NOISE;
NOISE;
REGISTER(C);
NOISE;
REGISTER(I);
NOISE;
REGISTER(A);
NOISE;
REGISTER(T);
REGISTER(B);
REGISTER(J);
REGISTER(K);
NOISE;
NOISE;
NOISE;
NOISE;
NOISE;
REGISTER(H);
REGISTER(E);
REGISTER(E);

int main()
{
    using namespace std;
    using namespace tmp;

    static_assert(is_same<CURRENT_TYPELIST,                          typelist<I, C, Z, G, D, F, L, C, I, A, T, B, J, K, H, E, E>>::value, MSG);
    static_assert(is_same<find_ancestors<CURRENT_TYPELIST, D>::type, typelist<A, C, D>>::value,                                           MSG);
    static_assert(is_same<find_ancestors<CURRENT_TYPELIST, K>::type, typelist<F, H, J, I, K>>::value,                                     MSG);

    D d_instance;
    SERIALIZE(d_instance);

    printf("\n\n");

    K k_instance;
    SERIALIZE(k_instance);

    return 0;
}


