#include "string.hpp"
typedef uint32_t u32;
typedef int32_t i32;

#define UNI_ALGO_DISABLE_PROP
#define UNI_ALGO_DISABLE_NORM
#define UNI_ALGO_DISABLE_COLLATION
#define UNI_ALGO_DISABLE_NFKC_NFKD
#define UNI_ALGO_DISABLE_UTF16
#define UNI_ALGO_STATIC_DATA

#define UNI_ALGO_IMPL_NAMESPACE_BEGIN
#define UNI_ALGO_IMPL_NAMESPACE_END
UNI_ALGO_IMPL_NAMESPACE_BEGIN
using type_codept = char32_t;
using type_char8  = unsigned char;
using type_char16 = char16_t;
using type_char32 = char32_t;
//const size_t impl_npos = static_cast<size_t>(-1); // the same as const size_t impl_npos = std::numeric_limits<size_t>::max();
//const std::nullptr_t impl_nullptr = nullptr;
static_assert(std::is_unsigned<type_codept>::value && sizeof(type_codept) >= sizeof(char32_t), "codept bad");
static_assert(std::is_unsigned<type_char8>::value, "char8 bad");
static_assert(std::is_unsigned<type_char16>::value && sizeof(type_char16) >= sizeof(char16_t), "char16 bad");
static_assert(std::is_unsigned<type_char32>::value && sizeof(type_char32) >= sizeof(char32_t), "char32 bad");
UNI_ALGO_IMPL_NAMESPACE_END
#define UNI_ALGO_DLL
#define UNI_ALGO_FORCE_C_ARRAYS
#include "lib/uni_algo/impl/impl_case.h"
string string::caseMapUtf8(int mode) const {
    const char* src = data();
    u32 len = length();
    string res((i32)(len * impl_x_case_map_utf8));
    char* dst = res.data();
    u32 dst_len = impl_case_map_utf8(
        src, src+len, 
        dst, mode
    );
    res.shrinkNonSubstringToFitLength(dst_len);
    return res;
}
string string::toUpperCase() const {
    return caseMapUtf8(
        impl_case_map_mode_uppercase
    );
}
string string::toTitleCase() const {
    return caseMapUtf8(
        impl_case_map_mode_titlecase
    );
}
string string::toLowerCase() const {
    return caseMapUtf8(
        impl_case_map_mode_lowercase
    );
}
string string::capitalize() const {
    const char* str = data();
    u32 len = length();
    u32 stop = len < 5 ? len : 5;
    u32 first_ascii = 0;
    while (first_ascii < stop 
    && (str[first_ascii] & 0x80)) {
        first_ascii++;
    }
    char buf[5 * impl_x_case_map_utf8];
    u32 buf_len = impl_case_map_utf8(
        str, str+first_ascii,
        buf, impl_case_map_mode_uppercase
    );
    return string(
        buf, str+first_ascii,
        buf_len, len-first_ascii
    );
}