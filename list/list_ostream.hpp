#ifndef LIST_OSTREAM_HPP
#define LIST_OSTREAM_HPP

#include <ostream>
#include <list>

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
    ostream& operator<<(ostream& os, const list<T>& l) {
        os << PREFIX;
        for (typename list<T>::iterator it = l.begin(); it != l.end(); ++it) {
            if (it != l.begin()) os << DELIMITER;
            os << *it;
        }
        os << POSTFIX;

        return os;
    }
}

#endif
