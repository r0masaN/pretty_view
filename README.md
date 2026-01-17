# collections_io_streams
Simple C++ library that complements 'std' by adding custom overloading for 'operator<<' in 'std::ostream' for standard collections ('vector', 'map', etc.) and more.

###
```c++
std::vector<int> vec{1, 2, 3, 4, 5};
rmsn::pretty_view pr_view{vec};
std::cout << pr_view;
```
```text
[1, 2, 3, 4, 5]
```

## Usage of the `pretty_view`
### 1. Including and files
Include pretty_view's header:
```c++
#include "pretty_view\pretty_view.hpp"
```
Or import module:
```c++
import pretty_view;
```
Module's file `pretty_view.ixx` is located at the same directory (with `pretty_view.hpp` and `pretty_view.tcc`, obviously).

### 2. Namespaces
If you don't want to write `rmsn::` every time:
```c++
using namespace rmsn;
```
Or, directly:
```c++
using rmsn::pretty_view;
```
There are 3 namespaces, by the way:
- `rmsn`: core `pretty_view` class and overloaded `operator<<` function,
- `rmsn::format`: prefixes, delimiters and postfixes for formatting output,
- `rmsn::detail`: hidden technical helping tools like concepts.

### 3. Formatting output
If you want to customize prefixes, delimiters and postfixes, use:
```c++
const char *rmsn::format::collection_prefix = "[";
const char *rmsn::format::collection_delimiter = ", ";
const char *rmsn::format::collection_postfix = "]";
const char *rmsn::format::tuple_prefix = "{";
const char *rmsn::format::tuple_delimiter = ", ";
const char *rmsn::format::tuple_postfix = "}";
```
You can set these `const char *` variables to whatever you want. It's just simple global state for any `pretty_view` object (will reinvent it in the future maybe), not (!) thread safe for now.
### 4. Wrapping structures
Wrap array/collection/tuple/etc. you want to print in `pretty_view` class:
```c++
std::vector<int> v{1, 2, 3, 4, 5}; // for example
rmsn::pretty_view pv{v};
```
You can also wrap structures of varying complex degrees of nesting:
```c++
std::map<int, std::tuple<int, std::vector<int>, std::string>> mega_map{
    {18, {10, {1, 2, 3}, "cat"}},
    {24, {48, {4, 51}, "dog"}}
};
rmsn::pretty_view pv1{mega_map};
// sometimes it's hard for compiler to deduce actual type of structure so you need to
// explicitly define it by yourself
rmsn::pretty_view<decltype<mega_map>> pv2{mega_map}; // the easiest way
rmsn::pretty_view<std::map<int, std::tuple<int, std::vector<int>, std::string>>> pv3{mega_map}; // the 💀 way
```
Wrapping is needed due to ADL mechanism in C++ which helps compiler to find correct `operator<<` overloading (mine). In simple words: adding new `operator<<` overloading in `std` namespace is unsafe and unreliable so we can have our own overloadings in ours namespaces. But you don't want write `rmsn::operator<<(std::cout, {1, 2, 3})`, right? That's when ADL appears: compiler sees `pretty_view` object from `rmsn` namespace and at first it looks at the current translation unit, after – at namespace `pretty_view` came from (which is `rmsn` where my `operator<<` overloading is placed).

### 5. Stream output
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
std::map<int, std::tuple<int, std::vector<int>, std::string>> mega_map{
    {18, {10, {1, 2, 3}, "cat"}},
    {24, {48, {4, 51}, "dog"}}
};
std::cout << rmsn::pretty_view{mega_map};
```
```text
[{18, {10, [1, 2, 3], cat}}, {24, {48, [4, 51], dog}}]
```
### 6. Support for non-STL data structures
The best thing is you can use your own collections or whatever you have coded, it just must satisfy the concepts `is_collection` or `is_tuple_like` from `rmsn::detail`. Good news is you don't need to care about scary things below unless you're going to write your own good-coded collection or data container.

There are simplified representations:
```c++
template<typename T>
concept is_collection = std::is_array_v<T> || requires (const T& t) {
    std::begin(t);
    std::end(t);
};

template<typename T>
concept is_tuple_like = requires {
    std::tuple_size<T>::value;
};
```
In the other words, if your something can provide `begin` and `end` iterators to iterate (collection-like) or can be used with `std::tuple_size_v<T>` and `std::get<I>(t)` – congratulations, you can use your innovation with `pretty_view` with much chill.

String-like objects (`std::string`, `std::string_view`, `char *`) are printed like strings, not like a collection of chars. By the way, you can use any char type C++ is provided. But notice that arrays of chars (`char arr[] = "Hello, World!"`) actually are printed as a collection (array) of chars.

### 7. Requirements
- C++ standard version 20+,
- G++/GCC (confirmed), MSVC & Clang/LLVM (potentially),
- `-fmodules` compilation flag for module usage,
- Windows, Linux, MacOS.
