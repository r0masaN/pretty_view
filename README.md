# collections_io_streams
Simple C++ library that complements 'std' by adding custom overloading for 'operator<<' in 'std::ostream' for standard collections ('vector', 'map', etc.) and more.

###
```c++
std::vector<int> vec{1, 2, 3, 4, 5};
std::cout << vec;
```
```text
[1, 2, 3, 4, 5]
```

## Usage of the `pretty_view`

### 0. Versions
For now, there are 2 versions – `v1` (`rmsn::pv::v1` namespace) and `v2` (`rmsn::pv::v2` namespace. **`v2` is much preferred.**

About differences – down through paragraphs.

### 1. Including and files
Include pretty_view's header:
```c++
#include "pretty_view_[VERSION]/pretty_view.hpp"
```
Or import module:
```c++
import pretty_view_[VERSION];
```
Module's file `pretty_view.ixx` is located at the same directory (with `pretty_view.hpp` and `pretty_view.tcc`, obviously).

### 2. Namespaces
If you don't want to write `rmsn::pv::` every time:
```c++
using namespace rmsn::pv;
```
There are 3 namespaces, by the way:
- `rmsn::pv::[VERSION]`: core `pretty_view` class and overloaded `operator<<` function,
- `rmsn::pv::[VERSION]::format`: prefixes, delimiters and postfixes for formatting output,
- `rmsn::pv::[VERSION]::detail`: hidden technical helping tools like concepts.

### 3. Formatting output
If you want to customize prefixes, delimiters and postfixes, use:
```c++
rmsn::pv::[VERSION]::format::collection_prefix = "[";
rmsn::pv::[VERSION]::format::collection_delimiter = ", ";
rmsn::pv::[VERSION]::format::collection_postfix = "]";
rmsn::pv::[VERSION]::format::tuple_prefix = "{";
rmsn::pv::[VERSION]::format::tuple_delimiter = ", ";
rmsn::pv::[VERSION]::format::tuple_postfix = "}";
```
Default values are demonstrated above. You can set these variables to whatever you want. It's just simple global state for any `pretty_view` object (will reinvent it in the future maybe), not (!) thread safe for now.

### 4. Wrapping structures

#### 4.1. `v1` version
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
Wrapping is needed due to ADL mechanism in C++ which helps compiler to find correct `operator<<` overloading (mine).

In simple words: adding new `operator<<` overloading in `std` namespace is unsafe and unreliable so we can have our own overloadings in ours namespaces. But you don't want write `rmsn::pv::v1::operator<<(std::cout, {1, 2, 3})`, right? That's when ADL appears: compiler sees `pretty_view` object from `rmsn::pv::v1` namespace and at first it looks at the current translation unit, after – at namespace `pretty_view` came from (which is `rmsn::pv::v1` where my `operator<<` overloading is placed).

#### 4.2. `v2` version
Luckily, `v2` version fixed that annoying linkage to wrapper structure aka proxy `pretty_view`. I just figured out that ADL not necessary in this situation, you can just write
```c++
#include "pretty_view_v1/pretty_view"

using namespace rmsn::pv::v2;
```
and with no troubles use my custom `operator<<` overloading because it was written in such way that compiler will find it and 100% add to Candidate Function Set without any ambiguous calls to standard `operator<<` overloadings from `std` namespace. So, no worries about it or whatever.

### 5. Stream output

#### 5.1. `v1` version
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

#### `v2` version
As long as I got rid of proxy class, no needed to wrap data structure into something. Just pass it into `operator<<`:
```c++
std::cout << v{1, 2, 3, 4, 5} << std::endl;

std::map<int, std::tuple<int, std::vector<int>, std::string>> mega_map{
    {18, {10, {1, 2, 3}, "cat"}},
    {24, {48, {4, 51}, "dog"}}
};
std::cout << mega_map;
```
```text
[1, 2, 3, 4, 5]
[{18, {10, [1, 2, 3], cat}}, {24, {48, [4, 51], dog}}]
```

### 6. Support for non-STL data structures
The best thing is you can use your own collections or whatever you have coded, it just must satisfy the concept `is_collection_or_tuple_and_not_string_like` from `rmsn::pv::[VERSION]::detail` (all `pretty_view` have same concept realization, luckily).

Good news is you don't need to care about scary things below unless you're going to write your own good-coded collection or data container.

There are simplified representations:
```c++
template<typename T>
concept is_collection_or_tuple_and_not_string_like =
        (is_collection<T> // is given type a collection or array
        || is_tuple_like<T>) // or is it a tuple or tuple-like (pair, for example)
        && !is_string_like<T>; // and is it not a string-like (string, string_view, raw char array etc.)
```
If your something is satisfying it – congratulations, you can use your invention with `pretty_view` with much chill.

String-like objects (`std::string`, `std::string_view`, `char *`) are printed like strings (using standard `operator<<` overloading from `std` namespace), not like a collection of chars. By the way, you can use any char type C++ is provided.

Also, of course, you can have any data structure with your custom types and `pretty_view` will correctly use your custom `operator<<` overloading for it. Quick example:
```c++
struct MyStruct {
    int i;
    std::string s;
    
    friend std::ostream& operator<<(std::ostream& os, const MyStruct& ms) {
        os << "MyStruct{" << i << ", \"" << s << "\"}";
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

### 7. Requirements
- C++ standard version 20+ (23+ for modules),
- G++/GCC (confirmed), MSVC & Clang/LLVM (potentially),
- `-fmodules` compilation flag for module usage,
- Windows, Linux, MacOS.
