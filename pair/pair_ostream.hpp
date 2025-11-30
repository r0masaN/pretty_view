#ifndef PAIR_OSTREAM_HPP
#define PAIR_OSTREAM_HPP

#include <ostream>
#include <bits/stl_pair.h>

#ifndef PREFIX
#define PREFIX "{"
#endif

#ifndef DELIMITER
#define DELIMITER ": "
#endif

#ifndef POSTFIX
#define POSTFIX "}"
#endif

namespace std {
    template<typename K, typename V>
    ostream& operator<<(ostream& os, const pair<K, V>& p) {
        os << PREFIX << p->first << DELIMITER << p->second << POSTFIX;

        return os;
    }
}

#endif
