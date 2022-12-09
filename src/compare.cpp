#include "string.hpp"

typedef uint32_t u32;

inline static u32 min(u32 a, u32 b) {
    if (a < b) {
        return a;
    }

    return b;
}

int string::compareInternal(const char* other, u32 other_slen) const {
    u32 slen = length();
    int res = memcmp(data(), other, min(slen, other_slen));
    if (res == 0) {
        if (slen < other_slen) {
            return -1;
        } else if (slen > other_slen) {
            return 1;
        } else {
            return 0;
        }
    }
    return res;
}