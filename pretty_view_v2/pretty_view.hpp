#ifndef PRETTY_VIEW_HPP
#define PRETTY_VIEW_HPP

#include <concepts>
#include <type_traits>
#include <string>
#include <string_view>
#include <tuple>
#include <ostream>

namespace rmsn::pv::v2::detail { // inner namespace for helping tools
    // get pure, clear type (without const, volatile qualifiers, reference)
    template<typename T>
    using base_t = std::remove_cvref_t<T>;

    // literally determines if given type is char-like (helping concept for is_string_like)
    template<typename T, typename BaseT = base_t<T>>
    concept is_char_like = std::same_as<BaseT, char> || std::same_as<BaseT, wchar_t> ||
        std::same_as<BaseT, char8_t> || std::same_as<BaseT, char16_t> || std::same_as<BaseT, char32_t>;

    // is the type is a string-like (need to ensure that we won't write a string-like type char by char like it's array of chars)
    // also used to prevent ambiguous call to standard `operator<<` overloadings in `std` for `const CharT *`-like types
    template<typename T, typename BaseT = base_t<T>>
    concept is_string_like = (std::is_array_v<BaseT> || std::is_pointer_v<BaseT>) &&
        is_char_like<decltype(std::declval<BaseT&>()[0])> || // (any char)-like pointer or array
        is_char_like<typename BaseT::value_type> && // or is it (any char)-based std::basic_string or std::basic_string_view
        (std::same_as<BaseT, std::basic_string<typename BaseT::value_type>> ||
        std::same_as<BaseT, std::basic_string_view<typename BaseT::value_type>>);

    // is the type is a collection-like (has iterators)
    template<typename T, typename BaseT = base_t<T>>
    concept is_collection = std::is_array_v<BaseT> || requires {
        std::begin(std::declval<BaseT&>()); // unified way to get an iterator instead of simple t.begin() that, for example, couldn't be
        std::end(std::declval<BaseT&>()); // invoked on raw arrays
    };

    // is the type is a tuple-like (can invoke std::tuple_size<T>::value => must have std::get<N>())
    template<typename T, typename BaseT = base_t<T>>
    concept is_tuple_like = requires {
        std::tuple_size<BaseT>::value;
    };
}

namespace rmsn::pv::v2::format { // global variables used in overloaded `operator<<`
    inline constinit const char *collection_prefix = "[", *collection_postfix = "]", *collection_delimiter = ", ",
        *tuple_prefix = "{", *tuple_postfix = "}", *tuple_delimiter = ", ";
}

namespace rmsn::pv::v2 { // main namespace
    // declaration of the `operator<<`
    template<typename T>
    requires (detail::is_collection<T> || detail::is_tuple_like<T>) && (!detail::is_string_like<T>)
    inline std::ostream& operator<<(std::ostream& os, const T& t);
}

#include "pretty_view.tcc" // realization of template method

#endif
