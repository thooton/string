#include "lib/utf8.h"
#include "lib/utf8/unchecked.h"
#include "string.hpp"

typedef uint8_t u8;
typedef uint32_t u32;
typedef int32_t i32;

bool string::isUtf8() const {
    const u8* start = (const u8*)data();
    const u8* end = start+length();
    return utf8::is_valid(start, end);
}
static u8* appendReplacement(u8* out) {
    // UTF-8 encoding of replacement character
    // (U+FFFD)
    *(out++) = 239;
    *(out++) = 191;
    *(out++) = 189;
    return out;
}
static u8* betterReplaceInvalid(const u8* start, const u8* end, u8* out) {
    while (start != end) {
        const u8* sequence_start = start;
        utf8::internal::utf_error err_code = utf8::internal::validate_next(start, end);
        switch (err_code) {
            case utf8::internal::UTF8_OK :
                for (const u8* it = sequence_start; it != start; ++it)
                    *out++ = *it;
                break;
            case utf8::internal::NOT_ENOUGH_ROOM:
                return out;
            case utf8::internal::INVALID_LEAD:
                out = appendReplacement(out);
                ++start;
                break;
            case utf8::internal::INCOMPLETE_SEQUENCE:
            case utf8::internal::OVERLONG_SEQUENCE:
            case utf8::internal::INVALID_CODE_POINT:
                out = appendReplacement(out);
                ++start;
                // just one replacement mark for the sequence
                while (start != end && utf8::internal::is_trail(*start))
                    ++start;
                break;
        }
    }
    return out;
}
string string::toUtf8() const {
    const u8* start = (const u8*)data();
    u32 len = length();
    const u8* end = start+len;
    const u8* invalid = utf8::find_invalid(start, end);
    if (invalid == end) {
        return *this;
    }
    // if there are N invalid utf8 bytes, 
    // there could be N replacement characters of 3 bytes each.
    string res((i32)(len * 3));
    u8* res_start = (u8*)res.data();
    u8* res_ptr = res_start;
    u32 valid_before_len = invalid-start;
    printf("valid_before_len: %d\n", valid_before_len);
    if (valid_before_len) {
        memcpy(res_ptr, start, valid_before_len);
        res_ptr += valid_before_len;
    }
    res_ptr = betterReplaceInvalid(invalid, end, res_ptr);
    u32 res_len = res_ptr-res_start;
    printf("res len %d\n", res_len);
    res.shrinkNonSubstringToFitLength(res_len);
    return res;
}