#pragma once

#define VERSION "1.1"

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

    /* Check if a type is defined or not */
    template <typename T, size_t = sizeof(T)>
    auto is_class_complete(T*)  -> std::true_type;
    constexpr auto is_class_complete(...) -> std::false_type;

}

#define DECLARE_TL(_name)\
    namespace tmp {\
        /* Capture __COUNTER__ and set the initial value */\
        template<typename> struct _name##CAPTURE;\
        template<>         struct _name##CAPTURE<size_t> : std::integral_constant<size_t, __COUNTER__> { };\
        template<size_t> struct _name##TL;\
        template<> struct _name##TL<_name##CAPTURE<size_t>::value> {\
            using type = tmp::typelist<>;\
        };\
        template <size_t I>\
        using _name##DEFINED = decltype(is_class_complete(std::declval<_name##TL<I>*>()));\
        \
        /* Select the current typelist at index "I" */\
        template<size_t I, bool = std::is_same<std::true_type, _name##DEFINED<I>>::value>\
        struct _name##READ;\
        \
        template<size_t I>\
        struct _name##READ<I, true> {\
            using type = typename _name##TL<I>::type;\
        };\
        \
        template<size_t I>\
        struct _name##READ<I, false> {\
            using type = typename std::conditional< \
                (I > _name##CAPTURE<size_t>::value), /* Are there more specializations to check? */\
                typename _name##READ<I-1>::type,     /* yes */\
                tmp::typelist<>                      /* no => failed => empty typelist */\
            >::type;\
        };\
    }

#define READ_TL(_name) typename _name##READ<__COUNTER__>::type
#define _ADD_TL(_name, _class, _idx)\
    /* Define the current typelist at index _idx */\
    namespace tmp {\
        template<>\
        struct _name##TL<_idx> {\
            using previous = typename _name##READ<_idx-1>::type;\
            using type = typename push_back<_class, previous>::type;\
        };\
    }
#define ADD_TL(_name, _class) _ADD_TL(_name, _class, __COUNTER__)
