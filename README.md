> Level Advance
# Global type list creation

In the [**previous article**](https://github.com/galtza/hierarchy-inspector), we introduced the metaprogramming construct ***type list***. We learnt how to perform a series of basic operations like *push_back* and *push_front*, or more complex transformations like *max* or *filter*. Finally, we implemented a meta-function called ***find_ancestors*** that given a *type list* TL and a type T, it generates another *type list* containing the ancestors of the type T, in declaration order. 

However, we did not describe how to generate the original ***type list*** in the first place. In this article, we will be presenting a technique to construct those *type lists* as the compilation takes place. We will analyse the basic elements we need from the perspective of an imperative language and then translate that into the realm of template metaprogramming. 

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

```c++
#include <vector>

std::vector<int> g_list;
int main () {
    g_list.push_back(1);
    g_list.push_back(3);
    auto local = g_list;
    return 0;
}
```

In this piece of code, we **declare** a variable named *g_list*, we **modify** it by using *push_back* and we **read** the content that then we assign to another variable. 

In the context of templates, the name "variable" is a bit misleading as, due to its functional nature, there are no side effects, so all things are immutable. Nevertheless, for the sake of argument, we will maintain the name but with *meta* as a prefix.

So, how do we **declare** a meta-variable? The same way that we were dealing with values in the previous example, now we deal with types. Hence, a meta-variable holds types, and, in our particular case, *type lists*. One way to specify new types is by using the type alias [*using*](http://en.cppreference.com/w/cpp/language/type_alias). Let us define an empty *type list*:

```c++
using registry0 = tmp::typelist<>;
```

If we want to construct new *type lists* based on this one, we need to create new aliases. For instance:

```c++
using registry1 = typename tmp::push_back<registry0, int>::type;
using registry2 = typename tmp::push_back<registry1, char>::type;
using registry3 = typename tmp::push_back<registry2, float>::type;
```

In this piece of code, every new type is based on the previous one. Actually, the 4 operations (1 declare and 3 modifications) are middle steps towards the final goal which is a *type list* with 3 types: `tmp::typelist<int, char, float>`. Therefore, those 4 aliases can be considered as "entries" in the history of a meta-variable.

## History of a meta-variable

Consider the following: 

```c++
template<size_t IDX>
struct a_history;

template<> struct a_history<0> {
    using type = tmp::typelist<>;
};
```

This is the **history wrapper** for a meta-variable called ***a*** that will hold a *type list*. In the declaration, the non-type template parameter IDX is the index in the history of the meta-variable. For each new value, we will need a new specialization with IDX higher than the previous one. The initial value corresponds to the index 0 and it is an empty *type list*. 

If we would want to add 3 new elements to the meta-variable, we would need 3 new specializations, with index 1, 2 and 3 respectively:

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

Each new entry makes use of the previous one to construct a new *type list* by pushing a new type.

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

Notice that the specialization for the initial value is associated to the index `__COUNTER__` instead of the 0 from previous examples (obviously we cannot assure that `__COUNTER__` has not been expanded previously, hence the value might be different from 0). 

In order to declare a *type list* by using this macro, we do:

```c++
DECLARE_TL(meta_variable_1);
DECLARE_TL(meta_variable_2);
...
```

In order to **read** the latest value of the meta-variable we use the following macro:

```c++
#define READ_TL(_name) typename _name##_history<__COUNTER__ - 1>::type
```

Notice that we need to subtract 1 to `__COUNTER__`  because it is incremented after expansion, so we need to point back in one position to point to the actual last history slot.

Finally, a possible macro to **modify** the meta-variable is the addition of a new type to the *type list*:

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

We create a new specialization for the new index (`__COUNTER__`), but we need to invoke it only once otherwise, we would be creating holes. That is the reason that we need an auxiliary macro to do the job: `ADD_TL` grabs the new index from `__COUNTER__` and passes it to the auxiliary macro `_ADD_TL`.

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

This works: the invocations to `ADD_TL` could be just a class that is being registered as the code is processed by the compiler. 

## It works, does it?

Maybe you noticed that there are some flaws in this implementation:

- multiple `READ_TL` invocations do not work (won't compile)
- Invocations to `__COUNTER__` between any of the macros do not work (won't compile either)

Both problems are related to the creation of *holes* in the history of a meta-variable. As we call `READ_TL`, we invoke `__COUNTER__` that will increase the value of the counter, hence the next time we try to read the meta-variable, we will use an index that does not exist inside the history of the meta-variable.

The same problem will happen if we invoke `__COUNTER__` arbitrarily between calls to `ADD_TL`, because in that call we will try to grab the previous history entry that does not exist.

Despite the flaws, there is still hope. There is a simple solution: **allow holes**.

Basically, we need to be able to read the latest value by *jumping* holes backwards until we reach a not-hole entry or we get to the beginning of the history (Which remember that has the initial empty *type list*).

## Sizeof

`sizeof` operator is our salvation. One of the requirements to be a valid expression is that the type specified as a parameter is complete (not only declared: fully defined). Taking advantage of this we can build the following:

```c++
namespace tmp {

    // Check if a type is defined or not
    template <typename T, size_t = sizeof(T)>
    auto is_class_complete(T*) -> std::true_type;

    constexpr auto is_class_complete(...) -> std::false_type;

}
```

The function overloading mechanism will select the template function if `T` is defined, as the `sizeof(T)` will be a valid expression and this function is more specific than the non-template version, which is the fallback.

In addition, these functions return types that we can identify with a Boolean *true*, or *false* respectively. If `T` is not defined, the second function will be chosen and will return a type that we can identify with Boolean *false*.

## New macros

The **declaration** and **read** macros are rewritten like this (Check the tagged (1), (2) and (3)):

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
    /* (2) Check if the entry at "IDX" exists */\
    template <size_t IDX>\
    using _name##_is_defined = decltype(\
        tmp::is_class_complete(std::declval<_name##_history<IDX>*>())\
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
    }
```

At *(1)* we are doing the same thing as before with the only difference that we are now receiving the initial history index as a parameter. In addition, we are making use of an auxiliary macro which will use the index in several places (that is precisely the reason we need to capture first the current `__COUNTER__`) .

At *(2)* we are creating a type alias that will tell us if a history entry exists or not. This code is a bit difficult to understand and requires a bit of explanation. As we read from the inside to the outside:

- With `_name##_history<IDX>` we are referring to the history entry at index IDX.
- By using `std::declval` like this: `std::declval<_name##_history<I>*>()`, we are (informally speaking) pretending to create a pointer to that type. Formally, as `std::declval` is only declared and not defined it is only valid on unevaluated-contexts, which basically means that it only happens at compile time and it does not really create an object.
- Then, we use the previous as a parameter to the function `is_class_complete` which is inside a `decltype` specifier meaning again that we are in an unevaluated-context.
- At the top of everything is the alias of the previous. 

Finally, At *(3)* we will make use of *(2)* to have two specializations of the read meta-function. When it is true, we will just return the type associated with the history entry. When it is false, it will recursively try to go back in the history until it reaches the starting history index. In that case it will have failed, otherwise, it will end up in the previous case in which the entry did exist.

The rest of the macros are the same as before:

```c++
#define READ_TL(_name) typename _name##_read<__COUNTER__ - 1>::type
#define _ADD_TL(_name, _class, _idx)\
    /* Define the current typelist at index _idx */\
    template<>\
    struct _name##_history<_idx> {\
        using previous = typename _name##_read<_idx - 1>::type;\
        using type = typename tmp::push_back<_class, previous>::type;\
    }
#define ADD_TL(_name, _class) _ADD_TL(_name, _class, __COUNTER__)
```

## Put all together

This is a full example where you can see all the mentioned techniques:

```c++
#include <type_traits>
#include "tmp.h"

// Macros used to create a NOISE macro that forces __COUNTER__ to increase
#define _WRAP_PASTE(x,y) x##y
#define _MERGE(x,y) _WRAP_PASTE(x,y)
#define NOISE auto _MERGE(unused, __COUNTER__) = __COUNTER__

/*
   Class example hierarchies
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
```

#### About this document

March 27, 2018 &mdash; Raúl Ramos

[LICENSE](https://github.com/galtza/global-typelist/blob/master/LICENSE)
