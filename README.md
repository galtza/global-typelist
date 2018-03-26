# Global type lists

In the [**previous article**](https://github.com/galtza/hierarchy-inspector), we introduced the metaprogramming construct ***type list***. We learnt how to perform a series of basic operations on like *push_back* and *push_front*, or more complex transformations like *max* or *filter*. Finally, we implemented a meta-function called ***find_ancestors*** that given a *type list* TL and a type T, it generates another *type list* containing the ancestors of the type T, in declaration order. 

However, we did not talk about how to generate the **original raw *type list*** in the first place. Due to the functional nature of the template metaprogramming, it is difficult to find a way to build these lists other than just using "literals" like in here: `using registry = typename tmp::typelist<A, B>::type`.

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

In this piece of code, we **declare** a variable named *g_list*, we **modify** it by using *push_back* and we **read** the content. We need to find a way to do the same thing but with template metaprogramming.

## Now, templates

The name "variable" is a bit misleading in the context of template metaprogramming as, due to its functional nature, there are no side effects, so all is immutable. Nevertheless, for the sake of argument, we will continue using it but with "meta" as a prefix.

In any case, how do we **declare** a meta-variable? The same way that we were dealing with values before, now we will deal with types. Hence, a meta-variable holds types, particularly *type lists* in our case. One way to specify new types is by using the type alias [*using*](http://en.cppreference.com/w/cpp/language/type_alias). Let us define an empty *type list*:

```c++
using variable = tmp::typelist<>;
```

If we want to construct new *type lists* based on this one, we need to create new type aliases. For instance:

```cpp
using registry0 = tmp::typelist<>;
using registry1 = typename tmp::push_back<registry0, int>::type;
using registry2 = typename tmp::push_back<registry1, char>::type;
using registry3 = typename tmp::push_back<registry2, float>::type;
```

In this piece of code, every new type is based on the previous one. Actually, the 4 operations are middle steps towards the final goal which is a *type list* with 3 types: `tmp::typelist<int, char, float>`. Therefore, those 4 aliases can be considered as "entries" in the history of a meta-variable.

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
    using type = typename tmp::push_back<typename a_history<0>::type, int>::type;
};
template<> struct a_history<2> {
    using type = typename tmp::push_back<typename a_history<1>::type, char>::type;
};
template<> struct a_history<3> {
    using type = typename tmp::push_back<typename a_history<2>::type, float>::type;
};
```

Obviously this is not very useful yet. We have the problem of how to avoid to having to use the IDX every time we want to change a meta-variable and, as well, how to access to the actual value. Unfortunately, to solve this problem we need to use the preprocessor and a non-standard but fortunately very common macro called `__COUNTER__`. 

Quoted form [Clang documentation](https://clang.llvm.org/docs/LanguageExtensions.html#builtin-macros): 

> *`__COUNTER__` Defines to an integer value that starts at zero and is incremented each time the `__COUNTER__` macro is expanded*

So if we wrap all up inside macros we have:
```c++

```



