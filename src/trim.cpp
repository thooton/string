#include "string.hpp"
typedef uint32_t u32;
typedef int32_t i32;
typedef uint8_t u8;

/* whitespace is 0x1680, 0x2000-0x200a, 0x2028, 
                0x2029, 0x202f, 0x205f, 0x3000 */
static bool isUtf8Whitespace(
    u8 a, u8 b, u8 c
) {
    return (
        a == 226 
        && (
            (
                b == 128 
                && (
                    (
                        c >= 128 
                        && c <= 138
                    )
                    || c == 168 
                    || c == 169
                    || c == 175
                )
            ) 
            || (
                b == 129 
                && c == 159
            )
        )
    )
    || (
        a == 225
        && b == 154
        && c == 128
    ) 
    || (
        a == 227
        && b == 128
        && c == 128
    );
}

/* this is not strictly ASCII, since 
    0x85 (NEL) and 0xA0 (NBSP) are UTF-8. */
static inline bool isAsciiWhitespace(u8 ch) {
    return ch <= ' ' || ch == 0x85 || ch == 0xA0;
}

static u32 indexOfNonWhitespace(const char* str, u32 len) {
    u32 i = 0;
    while (i+2 < len) {
        if (isAsciiWhitespace(str[i])) {
            i++;
        } else if (isUtf8Whitespace(
            str[i], str[i+1], str[i+2]
        )) {
            i += 2;
        } else {
            return i;
        }
    }
    while (i < len) {
        if (isAsciiWhitespace(str[i])) {
            i++;
        } else {
            return i;
        }
    }
    return i;
}
static i32 indexOfNonWhitespaceRev(const char* str, u32 len) {
    i32 i = len;
    while (i >= 2) {
        if (isAsciiWhitespace(str[i])) {
            i--;
        } else if (isUtf8Whitespace(
            str[i-2], str[i-1], str[i]
        )) {
            i -= 2;
        } else {
            return i;
        }
    }
    while (i >= 0) {
        if (isAsciiWhitespace(str[i])) {
            i--;
        } else {
            return i;
        }
    }
    return i;
}

string string::trimLeft() const {
    u32 len = length();
    u32 start = indexOfNonWhitespace(data(), len);
    if (start == len) {
        return "";
    }
    return substring(start);
}
string string::trimRight() const {
    u32 len = length();
    i32 end = indexOfNonWhitespaceRev(data(), len);
    if (end == -1) {
        return "";
    }
    return substring(0, end+1);
}
string string::trim() const {
    u32 len = length();
    const char* str = data();
    u32 start = indexOfNonWhitespace(str, len);
    if (start == len) {
        return "";
    }
    i32 end = indexOfNonWhitespaceRev(str, len);
    if (end == -1) {
        return "";
    }
    return substring(start, end+1);
}