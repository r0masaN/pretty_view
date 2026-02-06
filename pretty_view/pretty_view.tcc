namespace rmsn::pv {
/**
 * The 1st way: <br>
 * - no "using rmsn::pv" needed; <br>
 * - pretty_view wrapper for collection/tuple needed; <br>
 * - operator<< overloading will be found due to ADL; <br>
 * - operator<< overloading will be selected due to concrete param type (pretty_view<...>).
*/

#if CPP_VERSION >= 202002L
    template<detail::is_collection_or_tuple_and_not_string_like T>
    pretty_view<T>::pretty_view(const T& t) noexcept : t_(t) {}
#elif CPP_VERSION >= 201103L
    template<typename T, typename Enable>
    pretty_view<T, Enable>::pretty_view(const T& t) noexcept : t_(t) {}
#endif

#if CPP_VERSION >= 201103L && CPP_VERSION < 201402L
    template<std::size_t... Is>
    struct index_sequence {};

    template<std::size_t N, std::size_t... Is>
    struct make_index_sequence_impl : make_index_sequence_impl<N - 1, N - 1, Is...> {};

    template<std::size_t... Is>
    struct make_index_sequence_impl<0, Is...> {
        using type = index_sequence<Is...>;
    };

    template<std::size_t N>
    using make_index_sequence = typename make_index_sequence_impl<N>::type;
#else
    using std::index_sequence;
    using std::make_index_sequence;
#endif

#if CPP_VERSION >= 201103L && CPP_VERSION < 201703L
    template<typename U>
    static void process_pretty_view(std::ostream& os, const pretty_view<U>& pv, std::true_type, std::false_type) {
        os << format::collection_prefix;

        const auto begin = std::begin(pv.t_), end = std::end(pv.t_);
        using elem_t = detail::base_t<decltype(*begin)>;

        for (auto it = begin; it != end; ++it) { // iterating on collection
            if (it != begin) os << format::collection_delimiter;
            // if collection's element is collection or tuple himself
            if (detail::is_collection<elem_t> || detail::is_tuple_like<elem_t>)
                process_pretty_view(os, pretty_view<elem_t>{*it},
                    std::integral_constant<bool, detail::is_collection<elem_t>>{},
                    std::integral_constant<bool, detail::is_tuple_like<elem_t>>{});
            else
                process_pretty_view(os, *it,
                    std::integral_constant<bool, detail::is_collection<elem_t>>{},
                    std::integral_constant<bool, detail::is_tuple_like<elem_t>>{});
        }

        os << format::collection_postfix;
    }

    template<typename U, std::size_t... I>
    static void print_tuple(std::ostream& os, const pretty_view<U>& pv, const index_sequence<I...>&);

    template<typename U>
    static void process_pretty_view(std::ostream& os, const pretty_view<U>& pv, std::false_type, std::true_type) {
        os << format::tuple_prefix;

        // here comes an index sequence
        print_tuple(os, pv, make_index_sequence<std::tuple_size<detail::base_t<U>>::value>{});

        os << format::tuple_postfix;
    }

    template<typename U, std::size_t... I>
    static void print_tuple(std::ostream& os, const pretty_view<U>& pv, const index_sequence<I...>&) {
        const volatile int dummy[] = {
            0,
            (
                ( // if that's the first element of tuple, don't write delimiter (before him)
                    I == 0 ? void() : void(os << format::tuple_delimiter),
                    [&os, &pv]() { // another anonymous lambda that does the same logic that 52-57 lines
                        const auto& elem = std::get<I>(pv.t_); // std::get<I>(pv.t_) gets an I-st element from tuple pv.baseT
                        using elem_t = detail::base_t<decltype(elem)>;
                        if (detail::is_collection<elem_t> || detail::is_tuple_like<elem_t>)
                            process_pretty_view(os, pretty_view<elem_t>{elem},
                                std::integral_constant<bool, detail::is_collection<elem_t>>{},
                                std::integral_constant<bool, detail::is_tuple_like<elem_t>>{});
                        else
                            process_pretty_view(os, elem,
                                std::integral_constant<bool, detail::is_collection<elem_t>>{},
                                std::integral_constant<bool, detail::is_tuple_like<elem_t>>{});
                    }() // immediately invoke this lambda
                ),
                0
            )...
        };

        (void) dummy;
    }

    template<typename T>
    static void process_pretty_view(std::ostream& os, const T& t, std::integral_constant<bool, false>, std::integral_constant<bool, false>) {
        os << t;
    }
#endif

#if CPP_VERSION >= 201703L && CPP_VERSION < 202002L
    template<typename U, std::size_t... I>
    static inline void print_tuple_pretty_view(std::ostream& os, const pretty_view<U>& pv, const std::index_sequence<I...>&) {
        ( // 35-41 lines will be applied for each unwrapped element from tuple
            ( // if that's the first element of tuple, don't write delimiter (before him)
                I == 0 ? void() : void(os << format::tuple_delimiter),
                    [&os, &pv]() { // another anonymous lambda that does the same logic that 52-57 lines
                        const auto& elem = std::get<I>(pv.t_); // std::get<I>(pv.t_) gets an I-st element from tuple pv.baseT
                        if constexpr (detail::is_collection_or_tuple_and_not_string_like<detail::base_t<decltype(elem)>>) os << pretty_view(elem);
                        else os << elem;
                    }() // immediately invoke this lambda
            ),
            ... // unwrapping variadic pack
        );
    }
#endif


    // realization of the operator<<
    template<typename U>
    inline std::ostream& operator<<(std::ostream& os, const pretty_view<U>& pv) {
#if CPP_VERSION >= 201703L
        if constexpr (detail::is_collection<U>) { // if proxy contains collection
            os << format::collection_prefix;

            const auto begin = std::begin(pv.t_), end = std::end(pv.t_);
            using elem_t = detail::base_t<decltype(*begin)>;

            for (auto it = begin; it != end; ++it) { // iterating on collection
                if (it != begin) os << format::collection_delimiter;
                // if collection's element is collection or tuple himself
                if constexpr (detail::is_collection_or_tuple_and_not_string_like<elem_t>) os << pretty_view(*it); // wrap in proxy => recursion
                else os << *it; // else it's primitive or class/struct that isn't baseT collection/tuple
            }

            os << format::collection_postfix;

        } else if constexpr (detail::is_tuple_like<U>) { // if proxy contains tuple
            os << format::tuple_prefix;


#if CPP_VERSION >= 202002L
            // fun :) it's anonymous lambda that's unwrapping index sequence made from tuple
            [&os, &pv]<std::size_t... I>(const std::index_sequence<I...>&) {
                ( // 35-41 lines will be applied for each unwrapped element from tuple
                    ( // if that's the first element of tuple, don't write delimiter (before him)
                        I == 0 ? void() : void(os << format::tuple_delimiter),
                        [&os, &pv]() { // another anonymous lambda that does the same logic that 52-57 lines
                            const auto& elem = std::get<I>(pv.t_); // std::get<I>(pv.t_) gets an I-st element from tuple pv.baseT
                            if constexpr (detail::is_collection_or_tuple_and_not_string_like<detail::base_t<decltype(elem)>>) os << pretty_view(elem);
                            else os << elem;
                        }() // immediately invoke this lambda
                    ),
                    ... // unwrapping variadic pack
                );
            }(std::make_index_sequence<std::tuple_size<detail::base_t<U>>::value>{}); // here comes an index sequence + immediately invocation
#elif CPP_VERSION >= 201703L
            // here comes an index sequence
            print_tuple_pretty_view(os, pv, std::make_index_sequence<std::tuple_size_v<detail::base_t<U>>>{});
#endif

            os << format::tuple_postfix;
        } // there are no other if-else branches cuz we work here only with collections and tuples
#elif CPP_VERSION >= 201103L
        process_pretty_view(os, pv,
            std::integral_constant<bool, detail::is_collection<U>>{},
            std::integral_constant<bool, detail::is_tuple_like<U>>{});
#endif

        return os;
    }


/**
 * The 2nd way: <br>
 * - "using rmsn::pv" needed; <br>
 * - no wrappers needed; <br>
 * - operator<< overloading will be found due to ADL; <br>
 * - operator<< overloading will be selected due to concept (is_collection_or_tuple_and_not_string_like<...>).
*/

#if CPP_VERSION >= 201103L && CPP_VERSION < 201703L
    template<typename T>
    static void process_collection_or_tuple(std::ostream& os, const T& t, std::true_type, std::false_type) {
        os << format::collection_prefix;

        const auto begin = std::begin(t), end = std::end(t);
        using elem_t = detail::base_t<decltype(*begin)>;

        for (auto it = begin; it != end; ++it) { // iterating on collection
            if (it != begin) os << format::collection_delimiter;
            process_collection_or_tuple(os, *it, detail::is_collection<elem_t>, detail::is_tuple_like<elem_t>);
        }

        os << format::collection_postfix;
    }

    template<typename T, std::size_t... I>
    static void print_tuple(std::ostream& os, const T& t, const index_sequence<I...>&);

    template<typename T>
    static void process_collection_or_tuple(std::ostream& os, const T& t, std::false_type, std::true_type) {
        os << format::tuple_prefix;

        // here comes an index sequence
        print_tuple(os, t, make_index_sequence<std::tuple_size<detail::base_t<T>>::value>{});

        os << format::tuple_postfix;
    }

    template<typename T, std::size_t... I>
    static void print_tuple(std::ostream& os, const T& t, const index_sequence<I...>&) {
        const volatile int dummy[] = {
            0,
            (
                ( // if that's the first element of tuple, don't write delimiter (before him)
                    I == 0 ? void() : void(os << format::tuple_delimiter),
                    [&os, &t]() { // another anonymous lambda that does the same logic that 52-57 lines
                        const auto& elem = std::get<I>(t); // std::get<I>(pv.t_) gets an I-st element from tuple pv.baseT
                        using elem_t = detail::base_t<decltype(elem)>;
                        process_collection_or_tuple(os, elem, detail::is_collection<elem_t>, detail::is_tuple_like<elem_t>);
                    }() // immediately invoke this lambda
                ),
                0
            )...
        };

        (void) dummy;
    }

    template<typename T>
    static void process_collection_or_tuple(std::ostream& os, const T& t, std::false_type, std::false_type) {
        os << t;
    }
#endif

#if CPP_VERSION >= 201703L && CPP_VERSION < 202002L
    template<typename T, std::size_t... I>
    static inline void print_tuple(std::ostream& os, const T& t, const std::index_sequence<I...>&) {
        ( // 24-27 lines will be applied for each unwrapped element from tuple
                ( // if that's the first element of tuple, don't write delimiter (before him)
                        I == 0 ? void() : void(os << format::tuple_delimiter),
                                os << std::get<I>(t) // std::get<I>(baseT) gets an I-st element from tuple baseT
                ),
                ... // unwrapping variadic pack
        );
    }
#endif

    // realization of the `operator<<`
#if CPP_VERSION >= 202002L
    template<detail::is_collection_or_tuple_and_not_string_like T>
#elif CPP_VERSION >= 201103L
    template<typename T, typename Enabled>
#endif
    inline std::ostream& operator<<(std::ostream& os, const T& t) {
#if CPP_VERSION >= 201703L
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

#if CPP_VERSION >= 202002L
            // fun :) it's anonymous lambda that's unwrapping index sequence made from tuple
            [&os, &t]<std::size_t... I>(const std::index_sequence<I...>&) {
                ( // 24-27 lines will be applied for each unwrapped element from tuple
                    ( // if that's the first element of tuple, don't write delimiter (before him)
                        I == 0 ? void() : void(os << format::tuple_delimiter),
                        os << std::get<I>(t) // std::get<I>(baseT) gets an I-st element from tuple baseT
                    ),
                    ... // unwrapping variadic pack
                );
            }(std::make_index_sequence<std::tuple_size_v<detail::base_t<T>>>{}); // here comes an index sequence + immediately invocation
#elif CPP_VERSION >= 201703L
            // here comes an index sequence
            print_tuple(os, t, std::make_index_sequence<std::tuple_size_v<detail::base_t<T>>>{});
#endif

            os << format::tuple_postfix;
        } // there are no other if-else branches cuz we work here only with collections and tuples
#elif CPP_VERSION >= 201103L
        process_collection_or_tuple(os, t, detail::is_collection<T>, detail::is_tuple_like<T>);
#endif

        return os;
    }
}
