# Global type lists

In the [**previous article**](https://github.com/galtza/hierarchy-inspector), we introduced the metaprogramming construct ***type list***. We learnt how to perform a series of basic operations like *push_back* and *push_front*, or more complex transformations like *max* or *filter*. Finally, we implemented a meta-function called ***find_ancestors*** that given a *type list* TL and a type T, it generates another *type list* containing the ancestors of the type T, in declaration order. 

However, we did not describe how to generate the original **raw *type list*** in the first place. Due to the functional nature of the template metaprogramming, it is difficult to find a way to build these lists other than just using "literals" like in: `using registry = typename tmp::typelist<A, B>::type`.

In this article we will be presenting a technique to construct those *type lists* as the compilation occurs. We will analyse the basics from the perspective of an imperative language and then translate that into template metaprogramming. 

All the article pieces of code will be based on the existence of *type lists* and the basic operation *push_back* as it is listed below:

```c++
namespace tmp {

    template<typename...>
    struct typelist {
    };

    template<typename T, typename TL> 
    struct push_back;

    template<typename T, typename...TS>
    struct push_back<T, typelist<TS...>> {
        using type = typelist<TS..., T>;
    };
    
}
```

## Imperative vs Functional

In any common imperative programming language, we are used to **declare**, **modify** and **read** the content of a variable. For instance:  

```cpp
#include <vector>

std::vector<int> g_list;
int main () {
    g_list.push_back(1);
    g_list.push_back(3);
    auto local = g_list;
    return 0;
}
```

In this piece of code we **declare** a variable named *g_list*, we **modify** it by using *push_back* and we **read** the content that then we assign to another variable. 

In the context of templates, the name "variable" is a bit misleading as, due to its functional nature, there are no side effects, so all things are immutable. Nevertheless, for the sake of argument, we will continue using it but with *meta* as a prefix.

In any case, how do we **declare** a meta-variable? The same way that we were dealing with values in the previous example, now we will deal with types. Hence, a meta-variable holds types, and, in our particular case, *type lists*. One way to specify new types is by using the type alias [*using*](http://en.cppreference.com/w/cpp/language/type_alias). Let us define an empty *type list*:

```c++
using registry0 = tmp::typelist<>;
```

If we want to construct new *type lists* based on this one, we need to create new type aliases. For instance:

```cpp
using registry1 = typename tmp::push_back<registry0, int>::type;
using registry2 = typename tmp::push_back<registry1, char>::type;
using registry3 = typename tmp::push_back<registry2, float>::type;
```

In this piece of code, every new type is based on the previous one. Actually, the 4 operations (1 declare and 3 modifications) are middle steps towards the final goal which is a *type list* with 3 types: `tmp::typelist<int, char, float>`. Therefore, those 4 aliases can be considered as "entries" in the history of a meta-variable.

## History of a meta-variable

Consider the following: 

```cpp
template<size_t IDX>
struct a_history;

template<> struct a_history<0> {
    using type = tmp::typelist<>;
};
```

This is the **history wrapper** for a meta-variable called ***a*** that will hold a *type list*. In the declaration, the non-type template parameter IDX is the index in the history of the meta-variable. For each new value we will need a new specialization with IDX higher than the previous one. The initial value corresponds to the index 0 and it is an empty *type list*. 

If we would want to add 3 new elements to the meta-variable, we will need 3 new specializations, with index 1, 2 and 3 respectively:

```c++
template<> struct a_history<1> {
    using type = typename tmp::push_back<int, typename a_history<0>::type>::type;
};
template<> struct a_history<2> {
    using type = typename tmp::push_back<char, typename a_history<1>::type>::type;
};
template<> struct a_history<3> {
    using type = typename tmp::push_back<float, typename a_history<2>::type>::type;
};
```

Each new entry makes use of the previous entry in the history so that we can *push_back* our new type.

Obviously, this is not very useful yet. We need a way to automatically refer to the previous entry and also a way to automatically generate the latest index to add to. 

In order to solve this problem we have to turn to the preprocessor and specifically to a non-standard but fortunately very common macro called `__COUNTER__`. 

Quoted form [Clang documentation](https://clang.llvm.org/docs/LanguageExtensions.html#builtin-macros): 

> *`__COUNTER__` Defined to an integer value that starts at zero and is incremented each time the `__COUNTER__` macro is expanded*

So, the **declaration** of a *type list* meta-variable that makes use of `__COUNTER__` would be like this:
```c++
#define DECLARE_TL(_name)\
    /* Declare the type list meta-variable and initialize with an empty list */\
    template<size_t IDX>\
    struct _name##_history;\
    \
    template<> struct _name##_history<__COUNTER__> {\
        using type = tmp::typelist<>;\
    }
```

Notice that the specialization for the initial value is associated to the index `__COUNTER__` instead of the 0 from previous examples (obviously we cannot assure that `__COUNTER__` has not been called previously, hence the value might be different from 0). 

In order to declare a *type list* by using this macro, we do:

```c++
DECLARE_TL(meta_variable_1);
DECLARE_TL(meta_variable_2);
...
```

To **read** the latest value of the meta-class we can use the following macro:

```c++
#define READ_TL(_name) typename _name##_history<__COUNTER__ - 1>::type
```

Notice that we need to subtract 1 to `__COUNTER__`  because it is a post-incremented macro so that after it has been used it changes the value: before invocation `i` and after invocation `i + 1`.

Finally, one possible macro to **modify** the meta-variable by adding a new type to the *type list* could be written like this:

```c++
#define _ADD_TL(_name, _class, _idx)\
    /* Create a new entry in the history at _idx */\
    template<>\
    struct _name##_history<_idx> {\
        using previous = typename _name##_history<_idx - 1>::type;\
        using type = typename tmp::push_back<_class, previous>::type;\
    }
#define ADD_TL(_name, _class) _ADD_TL(_name, _class, __COUNTER__)
```

We create a new specialization for the new index (`__COUNTER__`), but we need to invoke it only once otherwise we would be creating holes. That is the reason that we need an auxiliary macro to do the job: `ADD_TL` grabs the new index from `__COUNTER__` and passes it to the auxiliar macro `_ADD_TL`.

Then, inside `_ADD_TL` we grab the previous history entry and we use it to add a new type to the list. An example of usage could be this:

```c++
DECLARE_TL(registry);
ADD_TL(registry, int);
ADD_TL(registry, char);
ADD_TL(registry, float);

int main () {
    static_assert(is_same<READ_TL(registry), tmp::typelist<int, char, float>>::value, "Error");
    return 0;
}
```

This just works. The invocations to `ADD_TL` could be just classes being registered along the code. 

## It works, does it?

Maybe you noticed that there are some flaws in this implementation:

- multiple `READ_TL` invocations do not work
- Invocations to `__COUNTER__` between any of the macros do not work

Both problems are related to the creation of *holes* in the history of a meta-variable. As we call `READ_TL`, we invoke `__COUNTER__` that will increase the value of the counter, hence the next time we try to read the meta-variable, we will use an index that does not exist.

The same problem will happen if we invoke `__COUNTER__` between calls to `ADD_TL`, because it will try to grab the previous history entry.

Nevertheless, there is a solution which is **allowing holes**.

We need to modify the reading operation so that it skips the holes backwards towards the beginning of the history.

## Sizeof

`sizeof` operator is our salvation as one of the requirements to be a valid expression is that the type is complete, which means that it is not only declared but fully defined. The following code will rely on that to get to know if a type is defined or not:

```c++
// Check if a type is defined or not
template <typename T, size_t = sizeof(T)>
auto is_class_complete(T*) -> std::true_type;

constexpr auto is_class_complete(...) -> std::false_type;
```

The function overloading mechanism, will select the template function if `T` is defined, as the `sizeof(T)` will be a valid expression and this function is more specific than the non-template version.

In addition, this function returns a type that we can identify with a Boolean *true*. If `T` is not defined, the second function will be chosen and will return a type that we can identify with Boolean *false*.

## New macros

Let us recap first the things we need to adapt the macros to the new solution:

1. We need a new way of reading the latest value of a meta-class that involves to *jump* history holes.
2. As a consequence of 1, we need to know the first index of a given meta-variable, so that 1 can stop

The declaration macro is now like this:

```c++
#define DECLARE_TL(_name) _DECLARE_TL(_name, __COUNTER__)\
#define _DECLARE_TL(_name, _start)\
    /* (1) Declare the type list meta-variable and initialize with an empty list */\
    template<size_t IDX>\
    struct _name##_history;\
    \
    template<> struct _name##_history<_start> {\
        using type = tmp::typelist<>;\
    };\
    \
    /* (2) Select the current typelist at index "IDX" */\
    template <size_t IDX>\
    using _name##_is_defined = decltype(\
        is_class_complete(std::declval<_name##_history<IDX>*>())\
    );\         
    \
    /* (3) Read from an index IDX */\
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
    };\
```

At (1) we are doing the same thing as before with the only different that we are now receiving the initial history index from parameter. 

At (2) we are creating a type alias that will tell us if a history entry exists or not. It is done with the previous `sizeof` trick we explained in the previous section. The type of the alias will be `std::true_type` or `std::false_type` depending on if the class is complete or not.

Finally At (3) we will make use of (2) to have two specializations of the read metafunction. When it is true, we will just return the type associated in the history entry. When it is false, it will recursively try to go back in the history until it reaches the starting history index. In that case it will have failed, otherwise it will end up in the previous case in which the entry did exist.

The rest of the macros are the same as before:

```c++
#define READ_TL(_name) typename _name##_read<__COUNTER__ - 1>::type
#define _ADD_TL(_name, _class, _idx)\
    /* Define the current typelist at index _idx */\
    template<>\
    struct _name##_history<_idx> {\
        using previous = typename _name##_read<_idx-1>::type;\
        using type = typename push_back<_class, previous>::type;\
    }
#define ADD_TL(_name, _class) _ADD_TL(_name, _class, __COUNTER__)
```

## Full example

Let's try a full example
