#ifndef PRETTY_VIEW_HPP
#define PRETTY_VIEW_HPP


// the version of C++ standard that is currently in use
#ifdef _MSVC_LANG
#define CPP_VERSION _MSVC_LANG
#else
#define CPP_VERSION __cplusplus
#endif

#if CPP_VERSION >= 202002L
#include <concepts>
#endif
#include <type_traits>
#include <utility>
#include <string>
#if CPP_VERSION >= 201703L
#include <string_view>
#endif
#include <tuple>
#include <ostream>


namespace rmsn { // main namespace
    namespace dtl { // hidden inner namespace for helping tools
#if CPP_VERSION >= 201103L && CPP_VERSION < 202002L
        template<typename T>
        struct remove_cvref { // helper struct for removing reference & const-volatile qualification from the type T
            using type = typename std::remove_cv<typename std::remove_reference<T>::type>::type;
        };
// C++20+
#else
        using std::remove_cvref; // helper struct for removing reference & const-volatile qualification from the type T
#endif

        // get pure, clear type (without const, volatile qualifiers, reference)
        template<typename T>
        using base_t = typename remove_cvref<T>::type;

        // literally determines if given type is char-like (helping concept for is_string_like)
        template<typename T, typename BaseT = base_t<T>>
#if CPP_VERSION >= 202002L
        concept is_char_like = std::same_as<BaseT, char> || std::same_as<BaseT, wchar_t> ||
            std::same_as<BaseT, char8_t> || std::same_as<BaseT, char16_t> || std::same_as<BaseT, char32_t>;
#elif CPP_VERSION >= 201402L
        static constexpr bool is_char_like = std::is_same<BaseT, char>::value || std::is_same<BaseT, wchar_t>::value ||
            std::is_same<BaseT, char16_t>::value || std::is_same<BaseT, char32_t>::value;
#elif CPP_VERSION >= 201103L
        struct is_char_like_s {
            static constexpr bool value = std::is_same<BaseT, char>::value || std::is_same<BaseT, wchar_t>::value ||
                std::is_same<BaseT, char16_t>::value || std::is_same<BaseT, char32_t>::value;
        };
#endif

#if CPP_VERSION < 201703L
        // helper alias & struct for testing whether typenames Ts... could be instanced in template
        template<typename...>
        struct make_void {
            typedef void type;
        };

        template<typename... Ts>
        using void_t = typename make_void<Ts...>::type;

        template<bool Condition>
        using bool_constant = std::integral_constant<bool, Condition>;
#elif CPP_VERSION < 202002L
        using std::void_t;
        using std::bool_constant;
#endif

#if CPP_VERSION >= 202002L
        // is the type is a string-like (need to ensure that we won't write a string-like type char by char like it's array of chars)
        // also used to prevent ambiguous call to standard `operator<<` overloadings in `std` for `const CharT *`-like types
        template<typename T, typename BaseT = base_t<T>>
        concept is_string_like = (std::is_array_v<BaseT> || std::is_pointer_v<BaseT>) &&
            is_char_like<decltype(std::declval<BaseT&>()[0])> || // (any char)-like pointer or array
            is_char_like<typename BaseT::value_type> && // or is it (any char)-based std::basic_string or std::basic_string_view
            (std::same_as<BaseT, std::basic_string<typename BaseT::value_type>> ||
            std::same_as<BaseT, std::basic_string_view<typename BaseT::value_type>>);
#elif CPP_VERSION >= 201103L
        // helper structs that test if the given type T satisfies needed constraints
        template<typename T>
        struct char_array_or_pointer_s : bool_constant<(std::is_array<T>::value || std::is_pointer<T>::value) &&
#if CPP_VERSION >= 201402L
            is_char_like<typename std::remove_pointer<typename std::decay<T>::type>::type>> {};
#else
            is_char_like_s<typename std::remove_pointer<typename std::decay<T>::type>::type>::value> {};
#endif

        template<typename T, typename = void>
        struct char_string_or_string_view_s : std::false_type {};

        template<typename T>
        struct char_string_or_string_view_s<T, void_t<typename T::value_type>> : bool_constant<
#if CPP_VERSION >= 201402L
                is_char_like<typename T::value_type> // or is it (any char)-based std::basic_string or std::basic_string_view
#else
                is_char_like_s<typename T::value_type>::value // or is it (any char)-based std::basic_string or std::basic_string_view
#endif
                && (std::is_same<T, std::basic_string<typename T::value_type>>::value
#if CPP_VERSION >= 201703L
                || std::is_same<T, std::basic_string_view<typename T::value_type>>::value
#endif
            )> {};

        // is the type is a string-like (need to ensure that we won't write a string-like type char by char like it's array of chars)
        // also used to prevent ambiguous call to standard `operator<<` overloadings in `std` for `const CharT *`-like types
        template<typename T, typename BaseT = base_t<T>>
#if CPP_VERSION >= 201402L
        static constexpr bool is_string_like = char_array_or_pointer_s<BaseT>::value || char_string_or_string_view_s<BaseT>::value;
#else
        struct is_string_like_s {
            static constexpr bool value = char_array_or_pointer_s<BaseT>::value || char_string_or_string_view_s<BaseT>::value;
        };
#endif
#endif

#if CPP_VERSION >= 202002L
        // is the type is a collection-like (has iterators)
        template<typename T, typename BaseT = base_t<T>>
        concept is_collection = requires (const BaseT& baseT) {
            std::begin(baseT); // unified way to get an iterator (instead of simple baseT.begin() that, for example, couldn'baseT be
            std::end(baseT); // invoked on raw arrays
        }; // cuz std::string, std::string_view, char *, const char * also can get iterators
#elif CPP_VERSION >= 201103L
        // helper structs that test if the given type T satisfies needed constraints
        template<typename T, typename = void>
        struct is_collection_s : std::false_type {};

        // unified way to get an iterator (instead of simple baseT.begin() that, for example, couldn't be invoked on raw arrays
        // cuz std::string, std::string_view, char *, const char * also can get iterators
        template<typename T>
        struct is_collection_s<T, void_t<decltype(std::begin(std::declval<T&>())), decltype(std::end(std::declval<T&>()))>> : std::true_type {};

#if CPP_VERSION >= 201402L
        // is the type is a collection-like (has iterators)
        template<typename T, typename BaseT = base_t<T>>
        static constexpr bool is_collection = is_collection_s<BaseT>::value;
#endif
#endif

#if CPP_VERSION >= 202002L
        // is the type is a tuple-like (can invoke std::get<N>(), std::tuple_size<T>)
        template<typename T, typename BaseT = base_t<T>>
        concept is_tuple_like = requires {
            std::tuple_size<BaseT>::value;
        };
#elif CPP_VERSION >= 201103L
        // helper structs that test if the given type T satisfies needed constraints
        template<typename T, typename = void>
        struct is_tuple_like_s : std::false_type {};

        template<typename T>
        struct is_tuple_like_s<T, void_t<decltype(std::tuple_size<T>::value)>> : std::true_type {};

#if CPP_VERSION >= 201402L
        // is the type is a tuple-like (can invoke std::get<N>(), std::tuple_size<T>)
        template<typename T, typename BaseT = base_t<T>>
        static constexpr bool is_tuple_like = is_tuple_like_s<BaseT>::value;
#endif
#endif

        // composite concept to not write chain of concepts every time
        template<typename T, typename BaseT = base_t<T>>
#if CPP_VERSION >= 202002L
        concept is_collection_or_tuple_and_not_string_like = (is_collection<BaseT> || is_tuple_like<BaseT>) &&
            !is_string_like<BaseT>;
#elif CPP_VERSION >= 201402L
        static constexpr bool is_collection_or_tuple_and_not_string_like = (is_collection<BaseT> || is_tuple_like<BaseT>) &&
            !is_string_like<BaseT>;
#elif CPP_VERSION >= 201103L
        struct is_collection_or_tuple_and_not_string_like_s {
            static constexpr bool value = (is_collection_s<BaseT>::value || is_tuple_like_s<BaseT>::value) &&
                !is_string_like_s<BaseT>::value;
        };
#endif
    }
}


namespace rmsn {
    namespace fmt { // global variables used in pretty_view.operator<<
        // helper struct for formatting collections and tuples output
        struct formatter {
            std::string collection_prefix = "[", collection_postfix = "]", collection_delimiter = ", ",
                tuple_prefix = "{", tuple_postfix = "}", tuple_delimiter = ", ";
        };

#if CPP_VERSION >= 202002L
        inline constinit
#elif CPP_VERSION >= 201703L
        inline
#endif
            formatter format;
    }
}


namespace rmsn {
/**
 * The 1st way: <br>
 * - no "using rmsn::pv" needed; <br>
 * - pretty_view wrapper for collection/tuple needed; <br>
 * - operator<< overloading will be found due to ADL; <br>
 * - operator<< overloading will be selected due to concrete param type (pretty_view<...>).
*/

    // declaration of the struct (to be visible in the following operator<<)
#if CPP_VERSION >= 202002L
    template<dtl::is_collection_or_tuple_and_not_string_like T>
#elif CPP_VERSION >= 201402L
    template<typename T, typename Enable = typename std::enable_if<dtl::is_collection_or_tuple_and_not_string_like<T>, void>::type>
#elif CPP_VERSION >= 201103L
    template<typename T, typename Enable = typename std::enable_if<dtl::is_collection_or_tuple_and_not_string_like_s<T>::value, void>::type>
#endif
    class pretty_view;

    // declaration of the operator<<
    template<typename U>
    inline std::ostream& operator<<(std::ostream& os, const pretty_view<U>& pv);

    // simple proxy class for collections and tuples (to prevent ADL from breaking overloadings or messing with smth in std)
#if CPP_VERSION >= 202002L
    template<dtl::is_collection_or_tuple_and_not_string_like T>
#elif CPP_VERSION >= 201103L
    template<typename T, typename Enable>
#endif
    //TODO ситуация, когда в конструктор передаётся rvalue (UB)
    // можно использовать std::conditional_t<!IsTemporary, const T&, const T>
    class pretty_view {
    public:
        explicit pretty_view(const T& t, const fmt::formatter& format = fmt::format) noexcept(true);

        pretty_view(const pretty_view<T>& other) = delete;

        pretty_view(pretty_view<T>&& other) = delete;

        pretty_view& operator=(const pretty_view<T>& other) = delete;

        pretty_view& operator=(pretty_view<T>&& other) = delete;

        // declaration that that operator<< is friend (has access to private fields)
        template<typename U>
        friend inline std::ostream& operator<<(std::ostream& os, const pretty_view<U>& pv);

    private:
        const dtl::base_t<T>& t_;
        const fmt::formatter& format_;
    };

#if CPP_VERSION >= 201703L
    template<typename T>
    pretty_view(T) -> pretty_view<T>;
#endif

/**
 * The 2nd way: <br>
 * - "using rmsn::pv" needed; <br>
 * - no wrappers needed; <br>
 * - operator<< overloading will be found due to ADL; <br>
 * - operator<< overloading will be selected due to concept (is_collection_or_tuple_and_not_string_like<...>).
*/

    // declaration of the `operator<<`
#if CPP_VERSION >= 202002L
    template<dtl::is_collection_or_tuple_and_not_string_like T>
#elif CPP_VERSION >= 201402L
    template<typename T, typename Enabled = typename std::enable_if<dtl::is_collection_or_tuple_and_not_string_like<T>, void>::type>
#elif CPP_VERSION >= 201103L
    template<typename T, typename Enable = typename std::enable_if<dtl::is_collection_or_tuple_and_not_string_like_s<T>::value, void>::type>
#endif
    inline std::ostream& operator<<(std::ostream& os, const T& t);
}

#include "pretty_view.tcc" // realization of template methods/functions

#endif
