#include "lib/utf8.h"
#include "lib/utf8/unchecked.h"
#include "string.hpp"

int string::cp2utf8(char* buf, char32_t cp) {
    if (utf8::internal::is_code_point_valid(cp)) {
        char* endp = utf8::unchecked::append(cp, buf);
        return endp - buf;
    } else {
        // UTF-8 encoding of replacement character
        // (U+FFFD)
        unsigned char* res = (unsigned char*)buf;
        res[0] = 239;
        res[1] = 191;
        res[2] = 189;
        return 3;
    }
}