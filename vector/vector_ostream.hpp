#ifndef VECTOR_OSTREAM_HPP
#define VECTOR_OSTREAM_HPP

#include <ostream>
#include <vector>

#ifndef PREFIX
#define PREFIX "["
#endif

#ifndef DELIMITER
#define DELIMITER ", "
#endif

#ifndef POSTFIX
#define POSTFIX "]"
#endif

namespace std {
    template<typename T>
    ostream& operator<<(ostream& os, const vector<T>& v) {
        os << PREFIX;
        for (typename vector<T>::iterator it = v.begin(); it != v.end(); ++it) {
            if (it != v.begin()) os << DELIMITER;
            os << *it;
        }
        os << POSTFIX;

        return os;
    }
}

#endif
