# Global type lists

In the [**previous article**](https://github.com/galtza/hierarchy-inspector), we introduced the metaprogramming construct ***type list***. We learnt how to perform a series of basic operations like *push_back* and *push_front*, or more complex transformations like *max* or *filter*. Finally, we implemented a meta-function called ***find_ancestors*** that given a *type list* TL and a type T, it generates another *type list* containing the ancestors of the type T, in declaration order. 

However, we did not describe how to generate the original **raw *type list*** in the first place. Due to the functional nature of the template metaprogramming, it is difficult to find a way to build these lists other than just using "literals" like in: `using registry = typename tmp::typelist<A, B>::type`.

In this article we will be presenting a technique to construct those *type lists* as the compilation occurs. We will analyse the basics from the perspective of an imperative language and then translate that into template metaprogramming. 

## Imperative view

Check out this C++ non-metaprogramming piece of code:  

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

In this piece of code, among others things, we **declare** a variable named *g_list*, we **modify** it by using *push_back* and we **read** the content. We need to find a way to do the same thing but with template metaprogramming.

## Now, templates

The name "variable" is a bit misleading in the context of template metaprogramming as, due to its functional nature, there are no side effects, so all is immutable. Nevertheless, for the sake of argument, we will continue using it but with *meta* as a prefix.

In any case, how do we **declare** a meta-variable? The same way that we were dealing with values before, now we will deal with types. Hence, a meta-variable holds types, particularly *type lists* in our case. One way to specify new types is by using the type alias [*using*](http://en.cppreference.com/w/cpp/language/type_alias). Let us define an empty *type list*:

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

This is a history wrapper for a meta-variable called ***a*** that will hold a *type list*. In the declaration, the non-type template parameter IDX is the index in the history of the meta-variable.  We have a specialization for **initializing** the history with the initial value (index 0) which will be an empty *type list* for us.

If we would want to add the types that we wanted in the previous example, we would do the following:

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

Obviously, this is not very useful yet. We have the problem of how to avoid having to use the IDX every time we want to change a meta-variable and, as well, how to access to the actual value. In order to solve this problem we have to turn to the preprocessor and specifically to a non-standard but fortunately very common macro called `__COUNTER__`. 

Quoted form [Clang documentation](https://clang.llvm.org/docs/LanguageExtensions.html#builtin-macros): 

> *`__COUNTER__` Defines to an integer value that starts at zero and is incremented each time the `__COUNTER__` macro is expanded*

So the declaration of a *type list* meta-variable that makes use of `__COUNTER__` would be like this:
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

There will be a type named `_name##_history` that will have one specialization per history entry. Notice that with the declaration we specify a specialization for the initial value that needs to be associated to the actual value of `__COUNTER__` at the time the macro is onvoked. This is done this way because obviously we cannot assure that `__COUNTER__` has not been called previously. 

In order to declare a *type list* by using this macro, we do:

```c++
DECLARE_TL(meta_variable_1);
DECLARE_TL(meta_variable_2);
...
```

In order to access to the latest value we define this macro:

```c++
#define READ_TL(_name) typename _name##_history<__COUNTER__ - 1>::type
```

Notice that we need to substract 1 to the `__COUNTER__` macro as it is post-incremented as it is used so that the actual value of `__COUNTER__` does not really give us access to any history entry.

Finally, the macro to add one new type to the meta-variable could be written like this:

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

In order to add a new element we create a new specialization for the new index (`__COUNTER__`), but we need to invoke it only once otherwise we would be creating holes. That is the reason that we need an auxiliary macro to do the job: `ADD_TL` grabs the new index from `__COUNTER__` and passes it to the auxiliar macro `_ADD_TL`.

Then, inside `_ADD_TL` we grab the previous value and use it to add a new type. An example of usage could be this:

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

This just works. The invocations `ADD_TL` could be classes being registered along the code. 

## Works, but...

Maybe you noticed that there are some flaws so far:

- multiple `READ_TL` invocations
- Invocations to `__COUNTER__`between any of the macros

All are 

















