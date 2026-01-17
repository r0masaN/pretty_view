export module pretty_view_v2;

import <concepts>;
import <type_traits>;
import <string>;
import <string_view>;
import <tuple>;
import <ostream>;

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
    concept is_collection = requires {
        std::begin(std::declval<BaseT&>()); // unified way to get an iterator instead of simple t.begin() that, for example, couldn't be
        std::end(std::declval<BaseT&>()); // invoked on raw arrays
    };

    // is the type is a tuple-like (can invoke std::tuple_size<T>::value => must have std::get<N>())
    template<typename T, typename BaseT = base_t<T>>
    concept is_tuple_like = requires {
        std::tuple_size<BaseT>::value;
    };

    // composite concept to not write chain of concepts every time
    template<typename T, typename BaseT = base_t<T>>
    concept is_collection_or_tuple_and_not_string_like = (detail::is_collection<BaseT> || detail::is_tuple_like<BaseT>) && !detail::is_string_like<BaseT>;
}

export namespace rmsn::pv::v2::format { // global variables used in overloaded `operator<<`
    inline constinit const char *collection_prefix = "[", *collection_postfix = "]", *collection_delimiter = ", ",
        *tuple_prefix = "{", *tuple_postfix = "}", *tuple_delimiter = ", ";
}

export namespace rmsn::pv::v2 {
    // declaration of the `operator<<`
    template<detail::is_collection_or_tuple_and_not_string_like T>
    inline std::ostream& operator<<(std::ostream& os, const T& t);

    // realization of the `operator<<`
    template<detail::is_collection_or_tuple_and_not_string_like T>
    inline std::ostream& operator<<(std::ostream& os, const T& t) {
        if constexpr (detail::is_collection<T>) { // if type is collection
            os << format::collection_prefix;

            const auto begin = std::begin(t), end = std::end(t);
            for (auto it = begin; it != end; ++it) { // iterating on collection
                if (it != begin) os << format::collection_delimiter;
                os << *it;
            }

            os << format::collection_postfix;

        } else if constexpr (detail::is_tuple_like<T>) { // if type is tuple
            os << format::tuple_prefix;

            // fun :) it's anonymous lambda that's unwrapping index sequence made from tuple
            [&os, &t]<std::size_t... I>(const std::index_sequence<I...>&) {
                ( // 24-27 lines will be applied for each unwrapped element from tuple
                        ( // if that's the first element of tuple, don't write delimiter (before him)
                                I == 0 ? void() : void(os << format::tuple_delimiter),
                                        [&os, &t]() { // another anonymous lambda that does the same logic that 52-57 lines
                                            os << std::get<I>(t); // std::get<I>(t) gets an I-st element from tuple t
                                        }() // immediately invoke this lambda
                        ),
                        ... // unwrapping variadic pack
                );
            }(std::make_index_sequence<std::tuple_size_v<detail::base_t<T>>>{}); // here comes an index sequence + immediately invocation

            os << format::tuple_postfix;
        } // there are no other if-else branches cuz we work here only with collections and tuples

        return os;
    }
}
