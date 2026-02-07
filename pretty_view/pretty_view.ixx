export module pretty_view;

import <concepts>;
import <type_traits>;
import <string>;
import <string_view>;
import <tuple>;
import <ostream>;

namespace rmsn::dtl { // inner namespace for helping tools
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
        std::begin(std::declval<BaseT&>()); // unified way to get an iterator (instead of simple t.begin() that, for example, couldn't be
        std::end(std::declval<BaseT&>()); // invoked on raw arrays
    }; // cuz std::string, std::string_view, char *, const char * also can get iterators

    // is the type is a tuple-like (can invoke std::get<N>(), std::tuple_size<T>)
    template<typename T, typename BaseT = base_t<T>>
    concept is_tuple_like = requires {
        std::tuple_size<BaseT>::value;
    };

    // composite concept to not write chain of concepts every time
    template<typename T, typename BaseT = base_t<T>>
    concept is_collection_or_tuple_and_not_string_like = (dtl::is_collection<BaseT> || dtl::is_tuple_like<BaseT>) &&
                                                         !dtl::is_string_like<BaseT>;
}


export namespace rmsn::fmt { // global variables used in pretty_view.operator<<
    inline constinit const char *collection_prefix = "[", *collection_postfix = "]", *collection_delimiter = ", ",
        *tuple_prefix = "{", *tuple_postfix = "}", *tuple_delimiter = ", ";
}


export namespace rmsn {
/**
 * The 1st way: <br>
 * - no "using rmsn::pv" needed; <br>
 * - pretty_view wrapper for collection/tuple needed; <br>
 * - operator<< overloading will be found due to ADL; <br>
 * - operator<< overloading will be selected due to concrete param type (pretty_view<...>).
*/

    // declaration of the struct (to be visible in the following operator<<)
    template<dtl::is_collection_or_tuple_and_not_string_like T>
    class pretty_view;

    // declaration of the operator<<
    template<typename U>
    std::ostream& operator<<(std::ostream& os, const pretty_view<U>& pv);

    // simple proxy class for collections and tuples (to prevent ADL from breaking overloadings or messing with some shit in std)
    // TODO proxy contains things from `format` => `operator<<` overloading becomes thread-safe
    template<dtl::is_collection_or_tuple_and_not_string_like T>
    class pretty_view {
    public:
        explicit pretty_view(const T& t) noexcept : t_(t) {}

        pretty_view(const pretty_view<T>& other) = delete;

        pretty_view(pretty_view<T>&& other) = delete;

        pretty_view& operator=(const pretty_view<T>& other) = delete;

        pretty_view& operator=(pretty_view<T>&& other) = delete;

        // declaration that that operator<< is friend (has access to private fields)
        template<typename U>
        friend std::ostream& operator<<(std::ostream& os, const pretty_view<U>& pv);

    private:
        const dtl::base_t<T>& t_;
    };

    // realization of the operator<<
    template<typename U>
    std::ostream& operator<<(std::ostream& os, const pretty_view<U>& pv) {
        if constexpr (dtl::is_collection<U>) { // if proxy contains collection
            os << fmt::collection_prefix;

            const auto begin = std::begin(pv.t_), end = std::end(pv.t_);
            using elem_t = dtl::base_t<decltype(*begin)>;

            for (auto it = begin; it != end; ++it) { // iterating on collection
                if (it != begin) os << fmt::collection_delimiter;
                // if collection's element is collection or tuple himself
                if constexpr (dtl::is_collection_or_tuple_and_not_string_like<elem_t>) os << pretty_view(*it); // wrap in proxy => recursion
                else os << *it; // else it's primitive or class/struct that isn't collection/tuple
            }

            os << fmt::collection_postfix;

        } else if constexpr (dtl::is_tuple_like<U>) { // if proxy contains tuple
            os << fmt::tuple_prefix;

            // fun :) it's anonymous lambda that's unwrapping index sequence made from tuple
            [&os, &pv]<std::size_t... I>(const std::index_sequence<I...>&) {
                ( // 35-41 lines will be applied for each unwrapped element from tuple
                        ( // if that's the first element of tuple, don't write delimiter (before him)
                                I == 0 ? void() : void(os << fmt::tuple_delimiter),
                                        [&os, &pv]() { // another anonymous lambda that does the same logic that 52-57 lines
                                            const auto& elem = std::get<I>(pv.t_); // std::get<I>(pv.t_) gets an I-st element from tuple pv.t
                                            if constexpr (dtl::is_collection_or_tuple_and_not_string_like<dtl::base_t<decltype(elem)>>) os << pretty_view(elem);
                                            else os << elem;
                                        }() // immediately invoke this lambda
                        ),
                        ... // unwrapping variadic pack
                );
            }(std::make_index_sequence < std::tuple_size_v < dtl::base_t<U>>>{}); // here comes an index sequence + immediately invocation

            os << fmt::tuple_postfix;
        } // there are no other if-else branches cuz we work here only with collections and tuples

        return os;
    }

/**
 * The 2nd way: <br>
 * - "using rmsn::pv" needed; <br>
 * - no wrappers needed; <br>
 * - operator<< overloading will be found due to ADL; <br>
 * - operator<< overloading will be selected due to concept (is_collection_or_tuple_and_not_string_like<...>).
*/

    // declaration of the `operator<<`
    template<dtl::is_collection_or_tuple_and_not_string_like T>
    inline std::ostream& operator<<(std::ostream& os, const T& t);

    // realization of the `operator<<`
    template<dtl::is_collection_or_tuple_and_not_string_like T>
    inline std::ostream& operator<<(std::ostream& os, const T& t) {
        if constexpr (dtl::is_collection<T>) { // if type is collection
            os << fmt::collection_prefix;

            const auto begin = std::begin(t), end = std::end(t);
            for (auto it = begin; it != end; ++it) { // iterating on collection
                if (it != begin) os << fmt::collection_delimiter;
                os << *it;
            }

            os << fmt::collection_postfix;

        } else if constexpr (dtl::is_tuple_like<T>) { // if type is tuple
            os << fmt::tuple_prefix;

            // fun :) it's anonymous lambda that's unwrapping index sequence made from tuple
            [&os, &t]<std::size_t... I>(const std::index_sequence<I...>&) {
                ( // 24-27 lines will be applied for each unwrapped element from tuple
                        ( // if that's the first element of tuple, don't write delimiter (before him)
                                I == 0 ? void() : void(os << fmt::tuple_delimiter),
                                        [&os, &t]() { // another anonymous lambda that does the same logic that 52-57 lines
                                            os << std::get<I>(t); // std::get<I>(t) gets an I-st element from tuple t
                                        }() // immediately invoke this lambda
                        ),
                        ... // unwrapping variadic pack
                );
            }(std::make_index_sequence < std::tuple_size_v < dtl::base_t<T>>>{}); // here comes an index sequence + immediately invocation

            os << fmt::tuple_postfix;
        } // there are no other if-else branches cuz we work here only with collections and tuples

        return os;
    }
}
