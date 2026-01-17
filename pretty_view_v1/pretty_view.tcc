namespace rmsn::pv::v1 {
    template<detail::is_collection_or_tuple_and_not_string_like T>
    constexpr pretty_view<T>::pretty_view(const T& t) noexcept : t_(t) {}

    template<detail::is_collection_or_tuple_and_not_string_like T>
    constexpr pretty_view<T>::pretty_view(const pretty_view<T>& other) noexcept : t_(other.t_) {}

    // realization of the operator<<
    template<typename U>
    inline std::ostream& operator<<(std::ostream& os, const pretty_view<U>& pv) {
        if constexpr (detail::is_collection<U>) { // if proxy contains collection
            os << format::collection_prefix;

            const auto begin = std::begin(pv.t_), end = std::end(pv.t_);
            using elem_t = detail::base_t<decltype(*begin)>;

            for (auto it = begin; it != end; ++it) { // iterating on collection
                if (it != begin) os << format::collection_delimiter;
                // if collection's element is collection or tuple himself
                if constexpr (detail::is_collection_or_tuple_and_not_string_like<elem_t>) os << pretty_view(*it); // wrap in proxy => recursion
                else os << *it; // else it's primitive or class/struct that isn't collection/tuple
            }

            os << format::collection_postfix;

        } else if constexpr (detail::is_tuple_like<U>) { // if proxy contains tuple
            os << format::tuple_prefix;

            // fun :) it's anonymous lambda that's unwrapping index sequence made from tuple
            [&os, &pv]<std::size_t... I>(const std::index_sequence<I...>&) {
                ( // 35-41 lines will be applied for each unwrapped element from tuple
                        ( // if that's the first element of tuple, don't write delimiter (before him)
                                I == 0 ? void() : void(os << format::tuple_delimiter),
                                        [&os, &pv]() { // another anonymous lambda that does the same logic that 52-57 lines
                                            const auto& elem = std::get<I>(pv.t_); // std::get<I>(pv.t_) gets an I-st element from tuple pv.t
                                            if constexpr (detail::is_collection_or_tuple_and_not_string_like<detail::base_t<decltype(elem)>>) os << pretty_view(elem);
                                            else os << elem;
                                        }() // immediately invoke this lambda
                        ),
                        ... // unwrapping variadic pack
                );
            }(std::make_index_sequence<std::tuple_size_v<detail::base_t<U>>>{}); // here comes an index sequence + immediately invocation

            os << format::tuple_postfix;
        } // there are no other if-else branches cuz we work here only with collections and tuples

        return os;
    }
}
