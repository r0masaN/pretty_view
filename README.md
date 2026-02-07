# pretty_view
C++ library that adds custom overloading for 'std::ostream& operator<<(std::ostream&, T)' for collections (`std::vector`, `std::map`, etc.) and tuple-like types (`std::tuple`, `std::pair`, etc.).

###
```c++
std::vector<int> vec{1, 2, 3, 4, 5};
std::cout << vec;
```
```text
[1, 2, 3, 4, 5]
```

## Usage of the `pretty_view`

### 0. Ways to use
For now, there are 2 ways – 1st is using wrapper class `pretty_view` and 2nd without anything additional.

About differences – down through paragraphs.

### 1. Including and files
Include pretty_view's header:
```c++
#include "pretty_view\pretty_view.hpp"
```
Or import module:
```c++
import pretty_view;
```
Module's file `pretty_view.ixx` is located at the same directory "pretty_view" as `pretty_view.hpp` and `pretty_view.tcc`.

### 2. Namespaces
If you don't want to write `rmsn::` every time:
```c++
using namespace rmsn;
```
There are 3 namespaces:
- `rmsn`: core `pretty_view` class and overloaded `operator<<` functions;
- `rmsn::fmt`: `formatter` class (with prefixes, delimiters and postfixes for formatting output) and global `formatter format` object (with default values);
- `rmsn::dtl`: hidden technical helping tools like concepts and type traits.

### 3. Formatting output
If you want to customize prefixes, delimiters and postfixes, use:
```c++
rmsn::fmt::format.collection_prefix = "[";
rmsn::fmt::format.collection_delimiter = ", ";
rmsn::fmt::format.collection_postfix = "]";
rmsn::fmt::format.tuple_prefix = "{";
rmsn::fmt::format.tuple_delimiter = ", ";
rmsn::fmt::format.tuple_postfix = "}";
```
Default values are demonstrated above. You can set these variables to whatever you want.

Of course, you can create `formatter` object and use it in `pretty_view` wrapper (pass it as second parameter – `pretty_view` will contain const reference to it); by default all `pretty_view` objects use `rmsn::fmt::format`.
```c++
rmsn::fmt::formatter custom_format{};
// ...
rmsn::pretty_view pv{/* container */, custom_format};
```

### 4. Wrapping structures

#### 4.1. 1st way
Wrap array/collection/tuple/etc. you want to print in `pretty_view` class:
```c++
std::vector<int> v{1, 2, 3, 4, 5}; // for example
rmsn::pretty_view pv{v, /* [optional] `formatter` object */};
```
You can also wrap structures of varying complex degrees of nesting:
```c++
std::map<int, std::tuple<int, std::vector<int>, std::string>> complex_map{
    {18, {10, {1, 2, 3}, "cat"}},
    {24, {48, {4, 51}, "dog"}}
};
rmsn::pretty_view pv1{complex_map};
// sometimes it's hard for compiler to deduce actual type of structure
// so you need to explicitly define it by yourself
rmsn::pretty_view<decltype<mega_map>> pv2{complex_map}; // the easiest way
rmsn::pretty_view<std::map<int, std::tuple<int, std::vector<int>, std::string>>> pv3{complex_map}; // the gigachad way
```
Wrapping is needed due to ADL mechanism in C++ which helps compiler to find correct `operator<<` overloading (mine) without explicit specifying a specific namespace.

In simple words: adding new `operator<<` overloading in `std` namespace is unsafe and unreliable so you can have our own overloadings in yours namespaces. But you don't want to write `rmsn::operator<<(std::cout, {1, 2, 3})`, do you? That's when ADL appears: compiler sees `pretty_view` object from `rmsn` namespace and at first it looks at the current translation unit, after – at namespace `pretty_view` came from (which is `rmsn` where my `operator<<` overloading is placed).

**Important!** Do not use temporary objects (both collection and formatter) in wrapper `pretty_view`, unless you instantly printing them. Lifetime of temporary objects ends at the end of the line they were declared!

Undefined behaviour:
```cpp
rmsn::pretty_view pv{
    std::vector<int>{1, 2, 3, 4, 5},
    rmsn::fmt::formatter{
        .collection_prefix = "<",
        .collection_postfix = ">"
    }
};
std::cout << pv;
```
Still safe but very close to death:
```cpp
std::cout << rmsn::pretty_view{
    std::vector<int>{1, 2, 3, 4, 5},
    rmsn::fmt::formatter{
        .collection_prefix = "<",
        .collection_postfix = ">"
    }
};
// because lifetime of temporary vector ends after operator<< performing
// it's still safe to print it
```
For you, it's better to use `pretty_view` wrapper with lvalue objects (objects that have name identifier and place in memory), not rvalue (temporary).

#### 4.2. 2nd way
Luckily, this way fixes that annoying linkage to wrapper structure `pretty_view`. I just figured out that ADL not that really necessary in this situation, you can just write:
```c++
#include "pretty_view\pretty_view.hpp"

using namespace rmsn;
```
and with no troubles use my custom `operator<<` overloading because it was written in such way that compiler will find it and 100% add to Candidate Function Set without any ambiguous calls to standard `operator<<` overloadings from `std` namespace.

### 5. Stream output

#### 5.1. 1st way
As usually with `std::cout`, you can use `operator<<` with any `std::ostream` you want, just pass into it `pretty_view` object:
```c++
std::vector<int> v{1, 2, 3, 4, 5};
rmsn::pretty_view pv{v};
std::ostream& os = std::cout; // just for example, you're free to use any possible std::ostream inheritor
os << pv;
```
```text
[1, 2, 3, 4, 5]
```
Enjoy the results :)
```c++
std::map<int, std::tuple<int, std::vector<int>, std::string>> complex_map{
    {18, {10, {1, 2, 3}, "cat"}},
    {24, {48, {4, 51}, "dog"}}
};
std::cout << rmsn::pretty_view{complex_map};
```
```text
[{18, {10, [1, 2, 3], cat}}, {24, {48, [4, 51], dog}}]
```

#### 2nd version
As long as I got rid of proxy class, no needed to wrap data structure into something. Just pass it into `operator<<`:
```c++
using namespace rmsn;

std::cout << std::vector<int>{1, 2, 3, 4, 5} << std::endl;

std::map<int, std::tuple<int, std::vector<int>, std::string>> complex_map{
    {18, {10, {1, 2, 3}, "cat"}},
    {24, {48, {4, 51}, "dog"}}
};
std::cout << complex_map;
```
```text
[1, 2, 3, 4, 5]
[{18, {10, [1, 2, 3], cat}}, {24, {48, [4, 51], dog}}]
```

### 6. Support for non-STL data structures
The best thing is you can use your own collections/tuple-likes or data containers from other libraries, they just must satisfy the concept `is_collection_or_tuple_and_not_string_like` from `rmsn::dtl`.

Good news is you don't need to care about scary things below unless you're going to write your own good-coded collection or tuple-like dta container.

There are simplified representations:
```c++
template<typename T>
concept is_collection_or_tuple_and_not_string_like =
        (is_collection<T> // is given type a collection or array
        || is_tuple_like<T>) // or is it a tuple or tuple-like (e.g. pair)
        && !is_string_like<T>; // and is it not a string-like (string, string_view, raw char array etc.)
```
If any data structure is satisfying this concept – congratulations, you can use it with `pretty_view` (or wrap-less) overloading. Otherwise, you will get compilation errors.

String-like objects consists of any `char` type (`std::string`, `std::string_view`, `std::basic_string<T>`, `char *`, etc.) will be printed like strings (using standard `operator<<` overloadings from `std` namespace), not like a collection of chars.

Also, of course, you can have any data structure with your custom types and `pretty_view` (or wrap-less) overloading will correctly use your custom `operator<<` overloading for it. Quick example:
```c++
struct MyStruct {
    int i;
    std::string s;
    
    friend std::ostream& operator<<(std::ostream& os, const MyStruct& ms) {
        os << "MyStruct{" << ms.i << ", \"" << ms.s << "\"}";
        return os;
    }
};

std::vector<MyStruct> my_structs{
    {5, "Jeff"},
    {1, "Brent"}
};
std::cout << my_structs;
```
```text
[MyStruct{5, "Jeff"}, MyStruct{1, "Brent"}]
```

### 7. Ongoing updates
- 3rd way (ostream wrapper);
- [maybe] 4th way (function that takes ostream, data structure and formatter).

### 8. Requirements
- **Standards:** C++11, C++14, C++17, C++20, C++23, C++23+;
- **Compilers:** G++/GCC (confirmed), MSVC & Clang/LLVM (potentially).
