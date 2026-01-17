namespace rmsn::pv::v2 {
    // realization of the `operator<<`
    template<typename T>
    requires (detail::is_collection<T> || detail::is_tuple_like<T>) && (!detail::is_string_like<T>)
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
