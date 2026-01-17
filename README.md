# collections_io_streams
Simple C++ library that complements 'std' by adding custom overloading for '&lt;&lt;' operators in 'iostream' for standard collections ('vector', 'map', etc.)

## Usage of the `pretty_view`
Include pretty_view's header:
```c++
#include "pretty_view\pretty_view.hpp"
```
Or import module:
```c++
import pretty_view;
```
Module's file `pretty_view.ixx` is located at the same directory.

If you don't want to write `rmsn::` every time:
```c++
using rmsn;
```
Or, directly:
```c++
using rmsn::pretty_view;
```

If you want to customize prefixes, delimiters and postfixes, use:
```c++
rmsn::format::collection_prefix = "[", rmsn::format::collection_delimiter = ", ", rmsn::format::collection_postfix = "]";
rmsn::format::tuple_prefix = "{", rmsn::format::tuple_delimiter = ", ", rmsn::format::tuple_postfix = "}";
```
You can set these `const char *` variables to whatever you want.

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
rmsn::pretty_view<decltype<mega_map>> pv2{mega_map};
rmsn::pretty_view<std::map<int, std::tuple<int, std::vector<int>, std::string>>> pv3{mega_map};
```

As usually with `std::cout`, you can use `operator<<` with any `std::ostream` you want, just pass into it `pretty_view` object:
```c++
std::vector<int> v{1, 2, 3, 4, 5};
rmsn::pretty_view pv{v};
std::ostream os;
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

The best thing is you can use your own collections or whatever you have coded, it just must satisfy the concepts `is_collection` or `is_tuple_like` from `rmsn::detail`, there are simplified representations:
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

String-like objects (`std::string`, `std::string_view`, `char *`) are printed like strings, not like a collection of chars. By the way, you can use any char type C++ is provided. By notice that arrays of chars (`char arr[] = "Hello, World!"`) actually are printed as a collection (array) of chars.
