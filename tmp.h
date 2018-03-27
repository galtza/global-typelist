#pragma once

#define VERSION "1.1"

namespace tmp {

    // type list
    template<typename...TS>
    struct typelist {
        static constexpr auto size = sizeof...(TS);
    };

    // basic operations
    template<typename T, typename TL> 
    struct push_back;

    template<typename T, typename...TS>
    struct push_back<T, typelist<TS...>> {
        using type = typelist<TS..., T>;
    };

    // Check if a type is defined or not
    template <typename T, size_t = sizeof(T)>
    auto is_class_complete(T*) -> std::true_type;
    constexpr auto is_class_complete(...) -> std::false_type;

}

#define DECLARE_TL(_name) _DECLARE_TL(_name, __COUNTER__)
#define _DECLARE_TL(_name, _start)\
    /* Declare the type list meta-variable and initialize with an empty list */\
    template<size_t IDX>\
    struct _name##_history;\
    \
    template<> struct _name##_history<_start> {\
        using type = tmp::typelist<>;\
    };\
    \
    /* Check if the entry at "IDX" exists */\
    template <size_t IDX>\
    using _name##_is_defined = decltype(\
        tmp::is_class_complete(std::declval<_name##_history<IDX>*>())\
    );\
    \
    /* Read from an index IDX */\
    template<size_t IDX, bool = std::is_same<std::true_type, _name##_is_defined<IDX>>::value>\
    struct _name##_read;\
    \
    template<size_t IDX>\
    struct _name##_read<IDX, true> {\
        using type = typename _name##_history<IDX>::type;\
    };\
    \
    template<size_t IDX>\
    struct _name##_read<IDX, false> {\
        using type = typename std::conditional< \
            (IDX > _start),                     /* Should we stop searching? */\
            typename _name##_read<IDX-1>::type, /* No */\
            tmp::typelist<>                     /* Yes => failed => empty typelist */\
        >::type;\
    }

#define READ_TL(_name) typename _name##_read<__COUNTER__ - 1>::type
#define _ADD_TL(_name, _class, _idx)\
    /* Define the current typelist at index _idx */\
    template<>\
    struct _name##_history<_idx> {\
        using previous = typename _name##_read<_idx - 1>::type;\
        using type = typename tmp::push_back<_class, previous>::type;\
    }
#define ADD_TL(_name, _class) _ADD_TL(_name, _class, __COUNTER__)
