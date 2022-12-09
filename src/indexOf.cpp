#include <limits.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

//#define Py_DEBUG

#if (ULONG_MAX >> 15) == 1
#define LONG_BIT 16
#elif (ULONG_MAX >> 31) == 1
#define LONG_BIT 32
#elif (ULONG_MAX >> 63) == 1
#define LONG_BIT 64
#elif (ULONG_MAX >> 128) == 1
#define LONG_BIT 128
#else
#error "Could not determine LONG_BIT"
#endif

#define Py_LOCAL_INLINE(ty) ty 
#define STRINGLIB(name) name
#define STRINGLIB_CHAR char
#ifdef Py_DEBUG
#define assert(x) ((!(x)) ? (puts("assert failed!\n"), exit(1), 0) : 0)
#else
#define assert(x)
#endif
#define _Py_ALIGN_DOWN(x, y) x
#define STRINGLIB_SIZEOF_CHAR 1
#define STRINGLIB_FAST_MEMCHR memchr
typedef int32_t Py_ssize_t;
#define Py_MAX(a, b) ((a) > (b)) ? (a) : (b)
#define Py_MIN(a, b) ((a) < (b)) ? (a) : (b)
#ifdef __cplusplus
#  define _Py_STATIC_CAST(type, expr) static_cast<type>(expr)
#else
#  define _Py_STATIC_CAST(type, expr) ((type)(expr))
#endif
#ifdef Py_DEBUG
#  define Py_SAFE_DOWNCAST(VALUE, WIDE, NARROW) \
       (assert(_Py_STATIC_CAST(WIDE, _Py_STATIC_CAST(NARROW, (VALUE))) == (VALUE)), \
        _Py_STATIC_CAST(NARROW, (VALUE)))
#else
#  define Py_SAFE_DOWNCAST(VALUE, WIDE, NARROW) _Py_STATIC_CAST(NARROW, (VALUE))
#endif
#include "lib/fastsearch.h"
#include "string.hpp"

typedef uint32_t u32;
typedef int32_t i32;

i32 string::indexOfInternal(const char* str, u32 str_len) const {
    return FASTSEARCH(
        data(), length(), 
        str, str_len, 
        1, FAST_SEARCH
    );
}
i32 string::indexOf(char ch) const {
    return find_char(data(), length(), ch);
}
i32 string::indexOf(char32_t cp) const {
    char buf[5];
    return indexOfInternal(
        buf, cp2utf8(buf, cp)
    );
}
i32 string::lastIndexOfInternal(const char* str, u32 str_len) const {
    return FASTSEARCH(
        data(), length(),
        str, str_len,
        1, FAST_RSEARCH
    );
}
i32 string::lastIndexOf(char ch) const {
    return rfind_char(data(), length(), ch);
}
i32 string::lastIndexOf(char32_t cp) const {
    char buf[5];
    return lastIndexOfInternal(
        buf, cp2utf8(buf, cp)
    );
}

u32 string::countOfInternal(const char* str, uint32_t str_len) const {
    return FASTSEARCH(
        data(), length(),
        str, str_len,
        INT32_MAX, FAST_COUNT
    );
}
u32 string::countOf(char ch) const {
    const char* s = data();
    const char* e = s+length();
    u32 count = 0;
    while (s < e) {
        if (*s == ch) {
            count++;
        }
        s++;
    }
    return count;
}
u32 string::countOf(char32_t cp) const {
    char buf[5];
    return countOfInternal(
        buf, cp2utf8(buf, cp)
    );
}

/* static */
int32_t string::stringlib_count(
    const char* hay, int32_t hlen, 
    const char* needle, int32_t nlen, int32_t maxcount
) {
    return FASTSEARCH(
        hay, hlen, needle, 
        nlen, maxcount, FAST_COUNT
    );
}

/* static */
int32_t string::stringlib_find(
    const char* hay, int32_t hlen,
    const char* needle, int32_t nlen,
    int32_t maxcount
) {
    return FASTSEARCH(
        hay, hlen, needle,
        nlen, maxcount, FAST_SEARCH
    );
}