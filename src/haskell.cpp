/*string drop(uint32_t);
    char head();
    string init();
    char last();
    string tail();
    string take(uint32_t);*/

#include "string.hpp"
typedef uint32_t u32;
string string::drop(u32 n) const {
    if (n >= length()) {
        return "";
    }
    return substring(n, length());
}
char string::head() const {
    return data()[0];
}
string string::init() const {
    u32 len = length();
    if (!len) {
        return *this;
    }
    return substring(0, len-1);
}
char string::last() const {
    u32 len = length();
    if (!len) {
        return 0;
    }
    return (*this)[length()-1];
}
