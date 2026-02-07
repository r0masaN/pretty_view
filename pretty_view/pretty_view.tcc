namespace rmsn {
#if CPP_VERSION >= 201103L && CPP_VERSION < 201402L
    // helper alias & struct for making compile-time index sequence
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
    // declaration of overloads of function for processing concrete type of data container

    // neither collection nor tuple-like
    template<typename T>
    static inline void process_collection_or_tuple(std::ostream& os, const T& t, [[maybe_unused]] const fmt::formatter& format, std::false_type, std::false_type);

    // not collection and tuple-like
    template<typename T>
    static inline void process_collection_or_tuple(std::ostream& os, const T& t, const fmt::formatter& format, std::false_type, std::true_type);

    // collection and not tuple-like
    template<typename T>
    static inline void process_collection_or_tuple(std::ostream& os, const T& t, const fmt::formatter& format, std::true_type , std::false_type);

    // collection and tuple-like (for example, std::array<T, std::size_t>)
    template<typename T>
    static inline void process_collection_or_tuple(std::ostream& os, const T& t, const fmt::formatter& format, std::true_type , std::true_type);

    // helper function for printing tuple
    template<typename T, std::size_t... I>
    static inline void print_tuple(std::ostream& os, const T& t, const fmt::formatter& format, const index_sequence<I...>&, [[maybe_unused]] bool is_collection);


    // just print
    template<typename T>
    static inline void process_collection_or_tuple(std::ostream& os, const T& t, [[maybe_unused]] const fmt::formatter& format, std::false_type, std::false_type) {
        os << t;
    }

    // process as tuple
    template<typename T>
    static inline void process_collection_or_tuple(std::ostream& os, const T& t, const fmt::formatter& format, std::false_type, std::true_type) {
        os << format.tuple_prefix;

        print_tuple(os, t, format, make_index_sequence<std::tuple_size<dtl::base_t<decltype(t)>>::value>{}, false);

        os << format.tuple_postfix;
    }

    // process as collection
    template<typename T>
    static inline void process_collection_or_tuple(std::ostream& os, const T& t, const fmt::formatter& format, std::true_type, std::false_type) {
        os << format.collection_prefix;

        const auto begin = std::begin(t), end = std::end(t);
        using elem_t = dtl::base_t<decltype(*begin)>;

        for (auto it = begin; it != end; ++it) { // iterating on collection
            if (it != begin) os << format.collection_delimiter;
            // recursive call to one of the overloadings
            process_collection_or_tuple(os, *it, format,
#if CPP_VERSION >= 201402L
                dtl::bool_constant<dtl::is_collection<elem_t> && !dtl::is_string_like<elem_t>>{},
                dtl::bool_constant<dtl::is_tuple_like<elem_t> && !dtl::is_string_like<elem_t>>{});
#elif CPP_VERSION >= 201103L
                dtl::bool_constant<dtl::is_collection_s<elem_t>::value && !dtl::is_string_like_s<elem_t>::value>{},
                dtl::bool_constant<dtl::is_tuple_like_s<elem_t>::value && !dtl::is_string_like_s<elem_t>::value>{});
#endif
        }

        os << format.collection_postfix;
    }

    // process as tuple, not like collection
    // because tuple is unpacked in compile time when collection – in runtime
    template<typename T>
    static inline void process_collection_or_tuple(std::ostream& os, const T& t, const fmt::formatter& format, std::true_type, std::true_type) {
        os << format.collection_prefix;

        // recursive call to one of the overloadings
        print_tuple(os, t, format, make_index_sequence<std::tuple_size<dtl::base_t<decltype(t)>>::value>{}, true);

        os << format.collection_postfix;
    }

    template<typename T, std::size_t... I>
    static inline void print_tuple(std::ostream& os, const T& t, const fmt::formatter& format, const index_sequence<I...>&, [[maybe_unused]] const bool is_collection) {
        // array trick to unpack parameters pack of indices (std::size_t... Is)
        // 'volatile' used to prevent the compiler from removing the dummy for optimizing purposes
        const volatile int dummy[] = {
            0,
            ( // 106-119 lines will be applied for each unwrapped element from tuple
                ( // if that's the first element of tuple, don't write delimiter (before him)
                    I == 0 ? void() : void(os << (is_collection ? format.collection_delimiter : format.tuple_delimiter)),
                    [&os, &t, &format]() { // another anonymous lambda that does the same logic that 52-57 lines
                        const auto& elem = std::get<I>(t); // std::get<I>(t.t_) gets an I-st element from tuple t.baseT
                        using elem_t = dtl::base_t<decltype(elem)>;
                        // recursive call to one of the overloadings
                        process_collection_or_tuple(os, elem, format,
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

        (void) dummy; // to prevent the compiler from removing the dummy for optimizing purposes
    }
#elif CPP_VERSION >= 201703L && CPP_VERSION < 202002L
    template<typename T, std::size_t... I>
    static inline void print_tuple(std::ostream& os, const T& t, const std::index_sequence<I...>&, const fmt::formatter format) {
        ( // 112-113 lines will be applied for each unwrapped element from tuple
            ( // if that's the first element of tuple, don't write delimiter (before him)
                I == 0 ? void() : void(os << (dtl::is_collection<T> ? format.collection_delimiter : format.tuple_delimiter)),
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
    pretty_view<T>::
#elif CPP_VERSION >= 201103L
    template<typename T, typename Enable>
    pretty_view<T, Enable>::
#endif
    pretty_view(const T& t, const fmt::formatter& format) noexcept(true) : t_(t), format_(format) {}

    // realization of the operator<<
    template<typename U>
    inline std::ostream& operator<<(std::ostream& os, const pretty_view<U>& pv) {
#if CPP_VERSION >= 201703L
        if constexpr (dtl::is_tuple_like<U>) { // if proxy contains tuple
            // if type is collection and tuple at a time (e.g. std::array), better to process it like tuple
            // because tuples unpack at a compile-time
            constexpr bool is_collection = dtl::is_collection<U>;

            if constexpr (is_collection) os << pv.format_.collection_prefix;
            else os << pv.format_.tuple_prefix;

#if CPP_VERSION >= 202002L
            // fun :) it's anonymous lambda that's unwrapping index sequence made from tuple
            [&os, &pv]<std::size_t... I>(const std::index_sequence<I...>&) {
                ( // 169-174 lines will be applied for each unwrapped element from tuple
                    ( // if that's the first element of tuple, don't write delimiter (before him)
                        I == 0 ? void() : void(os << (is_collection ? pv.format_.collection_delimiter : pv.format_.tuple_delimiter)),
                        [&os, &pv]() { // another anonymous lambda that does the same logic that 52-57 lines
                            const auto& elem = std::get<I>(pv.t_); // std::get<I>(pv.t_) gets an I-st element from tuple pv.baseT
                            if constexpr (dtl::is_collection_or_tuple_and_not_string_like<dtl::base_t<decltype(elem)>>) os << pretty_view{elem, pv.format_};
                            else os << elem;
                        }() // immediately invoke this lambda
                    ),
                    ... // unwrapping variadic pack
                );
            }(std::make_index_sequence<std::tuple_size<dtl::base_t<U>>::value>{}); // here comes an index sequence + immediately invocation
#else
            // here comes an index sequence
            print_tuple(os, pv.t_, std::make_index_sequence<std::tuple_size<dtl::base_t<U>>::value>{}, pv.format_);
#endif

            if constexpr (is_collection) os << pv.format_.collection_postfix;
            else os << pv.format_.tuple_postfix;

        } else if constexpr (dtl::is_collection<U>) { // if proxy contains collection
            os << pv.format_.collection_prefix;

            const auto begin = std::begin(pv.t_), end = std::end(pv.t_);
            using elem_t = dtl::base_t<decltype(*begin)>;

            for (auto it = begin; it != end; ++it) { // iterating on collection
                if (it != begin) os << pv.format_.collection_delimiter;
                // if collection's element is collection or tuple himself
                if constexpr (dtl::is_collection_or_tuple_and_not_string_like<elem_t>) os << pretty_view{*it, pv.format_}; // wrap in proxy => recursion
                else os << *it; // else it's primitive or class/struct that isn't baseT collection/tuple
            }

            os << pv.format_.collection_postfix;
        } // there are no other if-else branches cuz we work here only with collections and tuples
#elif CPP_VERSION >= 201103L
        // initial call to one of the overloadings
        process_collection_or_tuple(os, pv.t_, pv.format_,
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
        if constexpr (dtl::is_tuple_like<T>) { // if type is tuple
            // if type is collection and tuple at a time (e.g. std::array), better to process it like tuple
            // because tuples unpack at a compile-time
            constexpr bool is_collection = dtl::is_collection<T>;

            if constexpr (is_collection) os << fmt::format.collection_prefix;
            else os << fmt::format.tuple_prefix;

#if CPP_VERSION >= 202002L
            // fun :) it's anonymous lambda that's unwrapping index sequence made from tuple
            [&os, &t]<std::size_t... I>(const std::index_sequence<I...>&) {
                ( // 241-242 lines will be applied for each unwrapped element from tuple
                    ( // if that's the first element of tuple, don't write delimiter (before him)
                        I == 0 ? void() : void(os << (is_collection ? fmt::format.collection_delimiter : fmt::format.tuple_delimiter)),
                        os << std::get<I>(t) // std::get<I>(baseT) gets an I-st element from tuple baseT
                    ),
                    ... // unwrapping variadic pack
                );
            }(std::make_index_sequence<std::tuple_size_v<dtl::base_t<T>>>{}); // here comes an index sequence + immediately invocation
#else
            print_tuple(os, t, std::make_index_sequence<std::tuple_size<dtl::base_t<T>>::value>{}, fmt::format);
#endif

            if constexpr (is_collection) os << fmt::format.collection_postfix;
            else os << fmt::format.tuple_postfix;

        } else if constexpr (dtl::is_collection<T>) { // if type is collection
            os << fmt::format.collection_prefix;

            const auto begin = std::begin(t), end = std::end(t);
            for (auto it = begin; it != end; ++it) { // iterating on collection
                if (it != begin) os << fmt::format.collection_delimiter;
                os << *it;
            }

            os << fmt::format.collection_postfix;
        } // there are no other if-else branches cuz we work here only with collections and tuples
#elif CPP_VERSION >= 201103L
        // initial call to one of the overloadings
        process_collection_or_tuple(os, t, fmt::format,
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
