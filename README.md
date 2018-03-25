## Global type lists

### Introduction

In the [**previous article**](https://github.com/galtza/hierarchy-inspector), we introduced the metaprogramming construct ***type list***. We learnt how to perform a series of basic operations on it, like *push_back* and *push_front*, or more complex transformations like *max* or *filter*. Finally, we implemented a meta-function called ***find_ancestors*** that given a *type list* TL and a type T, it generates another *type list* containing the ancestors of the type T, in declaration order. 

However, we did not talk about how to generate the **original raw *type list*** in the first place. Due to the functional nature of the template metaprogramming, it is difficult to find a way to build these lists other than just using "literals" like in here: `using registry = typename tmp::typelist<A, B>::type`.

In this article we will be presenting a technique to construct those *type lists* as the compilation occurs. We will analyze the basics from the perspective of an imperative language and then translate that into template metaprogramming. 

### Imperative approach

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

### Template metaprogramming approach

The previous code was trivial and not very useful. But, in metaprogramming, how do we declare a meta-variable? The same way we were dealing with values before, we need now to work with types, specifically with *type lists*. The way of specifying types at compile time is using type alias. Let us define an empty *type list*:

```cpp
using variable = tmp::typelist<>;
```

The name "variable" is a bit misleading as we know that the same way that in functional programming there are no side effects, in template metaprogramming we cannot change a type once it is defined, but for the sake of argument, we will stay stick to the name "variable".

If we want to construct new *type lists* we need to base them on existing ones. For instance:

```cpp
using registry0 = tmp::typelist<>;
using registry1 = typename tmp::push_back<registry0, int>::type;
using registry2 = typename tmp::push_back<registry1, char>::type;
using registry3 = typename tmp::push_back<registry2, float>::type;
```

In this example, `registry0` is the base for constructing `registry1` and so on. We can store the whole history of changes of a variable.

### History of a *type list*

Imagine that we can store `registry0`, `registry1`, `registry2` and `registry3` in slots 0, 1, 2 and 3 respectively. We could get the latest value by accessing to the latest index (3 in this case). 

```cpp
template<size_t> struct history;\
template<> struct history<0> {\
    using type = tmp::typelist<>;\
};\
```

### Accessing to a variable





### aware of...

1. 










What if we have 
- tmp.h
- global.h
- a.h
- b.h
- m1.cpp
- m2.cpp

where a.h includes tmp.h and global.h and then registers a series of classes, then b.h does the same BUT m1.cpp includes first a.h and then b.h and m2 does the opposite? Could we find that we have two different type lists? 

Mention this in the article
::::::::::::



### Imperative approach

```cpp
#include <iostream>
#include <vector>
using namespace std;

vector<int> g_list;

void foo() {
    g_list.push_back(10);
}

void bar() {
    g_list.push_back(20);
}

int main () {
    foo ();
    bar ();
    for (auto v : g_list) {
        cout << v << "\n";
    }
    return 0;
}
```

### Identify elements

* Context of execution
* 



### Further reading

* [How to inspect a type hierarchy at compile-time](https://github.com/galtza/hierarchy-inspector)
* References:
  * [Initialization](http://en.cppreference.com/w/cpp/language/initialization)
