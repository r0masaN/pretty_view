namespace rmsn {
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
    template<typename T>
    static void process_pretty_view(std::ostream& os, const T& t, std::false_type, std::false_type);

    template<typename T>
    static void process_pretty_view(std::ostream& os, const T& t, std::false_type, std::true_type);

    template<typename T>
    static void process_pretty_view(std::ostream& os, const T& t, std::true_type , std::false_type);

    template<typename T>
    static void process_pretty_view(std::ostream& os, const T& t, std::true_type , std::true_type);

    template<typename T, std::size_t... I>
    static void print_tuple_pretty_view(std::ostream& os, const T& t, const index_sequence<I...>&);


    template<typename T>
    static void process_pretty_view(std::ostream& os, const T& t, std::false_type, std::false_type) {
        os << t;
    }

    template<typename T>
    static void process_pretty_view(std::ostream& os, const T& t, std::false_type, std::true_type) {
        os << fmt::tuple_prefix;

        // here comes an index sequence
        print_tuple_pretty_view(os, t, make_index_sequence<std::tuple_size<dtl::base_t<decltype(t)>>::value>{});

        os << fmt::tuple_postfix;
    }

    template<typename T>
    static void process_pretty_view(std::ostream& os, const T& t, std::true_type, std::false_type) {
        os << fmt::collection_prefix;

        const auto begin = std::begin(t), end = std::end(t);
        using elem_t = dtl::base_t<decltype(*begin)>;

        for (auto it = begin; it != end; ++it) { // iterating on collection
            if (it != begin) os << fmt::collection_delimiter;
            process_pretty_view(os, *it,
#if CPP_VERSION >= 201402L
                dtl::bool_constant<dtl::is_collection<elem_t> && !dtl::is_string_like<elem_t>>{},
                dtl::bool_constant<dtl::is_tuple_like<elem_t> && !dtl::is_string_like<elem_t>>{});
#elif CPP_VERSION >= 201103L
                dtl::bool_constant<dtl::is_collection_s<elem_t>::value && !dtl::is_string_like_s<elem_t>::value>{},
                dtl::bool_constant<dtl::is_tuple_like_s<elem_t>::value && !dtl::is_string_like_s<elem_t>::value>{});
#endif
        }

        os << fmt::collection_postfix;
    }

    template<typename T>
    static void process_pretty_view(std::ostream& os, const T& t, std::true_type, std::true_type) {
        process_pretty_view(os, t, dtl::bool_constant<true>{}, dtl::bool_constant<false>{});
    }

    template<typename T, std::size_t... I>
    static void print_tuple_pretty_view(std::ostream& os, const T& t, const index_sequence<I...>&) {
        const volatile int dummy[] = {
            0,
            (
                ( // if that's the first element of tuple, don't write delimiter (before him)
                    I == 0 ? void() : void(os << fmt::tuple_delimiter),
                    [&os, &t]() { // another anonymous lambda that does the same logic that 52-57 lines
                        const auto& elem = std::get<I>(t); // std::get<I>(t.t_) gets an I-st element from tuple t.baseT
                        using elem_t = dtl::base_t<decltype(elem)>;
                        process_pretty_view(os, elem,
#if CPP_VERSION >= 201402L
                            dtl::bool_constant<dtl::is_collection<elem_t> && !dtl::is_string_like<elem_t>>{},
                            dtl::bool_constant<dtl::is_tuple_like<elem_t> && !dtl::is_string_like<elem_t>>{});
#elif CPP_VERSION >= 201103L
                            dtl::bool_constant<dtl::is_collection_s<elem_t>::value && !dtl::is_string_like_s<elem_t>::value>{},
                            dtl::bool_constant<dtl::is_tuple_like_s<elem_t>::value && !dtl::is_string_like_s<elem_t>::value>{});
#endif
                    }() // immediately invoke this lambda
                ),
                0
            )...
        };

        (void) dummy;
    }

#elif CPP_VERSION >= 201703L && CPP_VERSION < 202002L
    template<typename T, std::size_t... I>
    static inline void print_tuple(std::ostream& os, const T& t, const std::index_sequence<I...>&) {
        ( // 24-27 lines will be applied for each unwrapped element from tuple
            ( // if that's the first element of tuple, don't write delimiter (before him)
                I == 0 ? void() : void(os << fmt::tuple_delimiter),
                os << std::get<I>(t) // std::get<I>(baseT) gets an I-st element from tuple baseT
            ),
            ... // unwrapping variadic pack
        );
    }
#endif


/**
 * The 1st way: <br>
 * - no "using rmsn::pv" needed; <br>
 * - pretty_view wrapper for collection/tuple needed; <br>
 * - operator<< overloading will be found due to ADL; <br>
 * - operator<< overloading will be selected due to concrete param type (pretty_view<...>).
*/

#if CPP_VERSION >= 202002L
    template<dtl::is_collection_or_tuple_and_not_string_like T>
    pretty_view<T>::pretty_view(const T& t) noexcept : t_(t) {}
#elif CPP_VERSION >= 201103L
    template<typename T, typename Enable>
    pretty_view<T, Enable>::pretty_view(const T& t) noexcept : t_(t) {}
#endif

#if CPP_VERSION >= 201703L && CPP_VERSION < 202002L
    template<typename U, std::size_t... I>
    static inline void print_tuple_pretty_view(std::ostream& os, const pretty_view<U>& pv, const std::index_sequence<I...>&) {
        ( // 35-41 lines will be applied for each unwrapped element from tuple
            ( // if that's the first element of tuple, don't write delimiter (before him)
                I == 0 ? void() : void(os << fmt::tuple_delimiter),
                    [&os, &pv]() { // another anonymous lambda that does the same logic that 52-57 lines
                        const auto& elem = std::get<I>(pv.t_); // std::get<I>(pv.t_) gets an I-st element from tuple pv.baseT
                        if constexpr (dtl::is_collection_or_tuple_and_not_string_like<dtl::base_t<decltype(elem)>>) os << pretty_view(elem);
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
        if constexpr (dtl::is_collection<U>) { // if proxy contains collection
            os << fmt::collection_prefix;

            const auto begin = std::begin(pv.t_), end = std::end(pv.t_);
            using elem_t = dtl::base_t<decltype(*begin)>;

            for (auto it = begin; it != end; ++it) { // iterating on collection
                if (it != begin) os << fmt::collection_delimiter;
                // if collection's element is collection or tuple himself
                if constexpr (dtl::is_collection_or_tuple_and_not_string_like<elem_t>) os << pretty_view{*it}; // wrap in proxy => recursion
                else os << *it; // else it's primitive or class/struct that isn't baseT collection/tuple
            }

            os << fmt::collection_postfix;

        } else if constexpr (dtl::is_tuple_like<U>) { // if proxy contains tuple
            os << fmt::tuple_prefix;

#if CPP_VERSION >= 202002L
            // fun :) it's anonymous lambda that's unwrapping index sequence made from tuple
            [&os, &pv]<std::size_t... I>(const std::index_sequence<I...>&) {
                ( // 35-41 lines will be applied for each unwrapped element from tuple
                    ( // if that's the first element of tuple, don't write delimiter (before him)
                        I == 0 ? void() : void(os << fmt::tuple_delimiter),
                        [&os, &pv]() { // another anonymous lambda that does the same logic that 52-57 lines
                            const auto& elem = std::get<I>(pv.t_); // std::get<I>(pv.t_) gets an I-st element from tuple pv.baseT
                            if constexpr (dtl::is_collection_or_tuple_and_not_string_like<dtl::base_t<decltype(elem)>>) os << pretty_view{elem};
                            else os << elem;
                        }() // immediately invoke this lambda
                    ),
                    ... // unwrapping variadic pack
                );
            }(std::make_index_sequence<std::tuple_size<dtl::base_t<U>>::value>{}); // here comes an index sequence + immediately invocation
#elif CPP_VERSION >= 201703L
            // here comes an index sequence
            print_tuple(os, pv.t_, std::make_index_sequence<std::tuple_size<dtl::base_t<U>>::value>{});
#endif

            os << fmt::tuple_postfix;
        } // there are no other if-else branches cuz we work here only with collections and tuples
#elif CPP_VERSION >= 201103L
        process_pretty_view(os, pv.t_,
#if CPP_VERSION >= 201402L
            dtl::bool_constant<dtl::is_collection<U> && !dtl::is_string_like<U>>{},
            dtl::bool_constant<dtl::is_tuple_like<U> && !dtl::is_string_like<U>>{});
#elif CPP_VERSION >= 201103L
            dtl::bool_constant<dtl::is_collection_s<U>::value && !dtl::is_string_like_s<U>::value>{},
            dtl::bool_constant<dtl::is_tuple_like_s<U>::value && !dtl::is_string_like_s<U>::value>{});
#endif
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

    // realization of the `operator<<`
#if CPP_VERSION >= 202002L
    template<dtl::is_collection_or_tuple_and_not_string_like T>
#elif CPP_VERSION >= 201103L
    template<typename T, typename Enabled>
#endif
    inline std::ostream& operator<<(std::ostream& os, const T& t) {
#if CPP_VERSION >= 201703L
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

#if CPP_VERSION >= 202002L
            // fun :) it's anonymous lambda that's unwrapping index sequence made from tuple
            [&os, &t]<std::size_t... I>(const std::index_sequence<I...>&) {
                ( // 24-27 lines will be applied for each unwrapped element from tuple
                    ( // if that's the first element of tuple, don't write delimiter (before him)
                        I == 0 ? void() : void(os << fmt::tuple_delimiter),
                        os << std::get<I>(t) // std::get<I>(baseT) gets an I-st element from tuple baseT
                    ),
                    ... // unwrapping variadic pack
                );
            }(std::make_index_sequence<std::tuple_size_v<dtl::base_t<T>>>{}); // here comes an index sequence + immediately invocation
#elif CPP_VERSION >= 201703L
            // here comes an index sequence
            print_tuple(os, t, std::make_index_sequence<std::tuple_size<dtl::base_t<T>>::value>{});
#endif

            os << fmt::tuple_postfix;
        } // there are no other if-else branches cuz we work here only with collections and tuples
#elif CPP_VERSION >= 201103L
        process_pretty_view(os, t,
#if CPP_VERSION >= 201402L
            dtl::bool_constant<dtl::is_collection<T> && !dtl::is_string_like<T>>{},
            dtl::bool_constant<dtl::is_tuple_like<T> && !dtl::is_string_like<T>>{});
#elif CPP_VERSION >= 201103L
            dtl::bool_constant<dtl::is_collection_s<T>::value && !dtl::is_string_like_s<T>::value>{},
            dtl::bool_constant<dtl::is_tuple_like_s<T>::value && !dtl::is_string_like_s<T>::value>{});
#endif
#endif

        return os;
    }
}
