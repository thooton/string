#include "string.hpp"

typedef uint32_t u32;
bool string::startsWithInternal(const char* str, u32 str_len) const {
    u32 len = length();
    if (len < str_len) {
        return false;
    }
    return memcmp(data(), str, str_len) == 0;
}
bool string::startsWith(char ch) const {
    return head() == ch;
}
bool string::startsWith(char32_t cp) const {
    char buf[5];
    return startsWithInternal(buf, cp2utf8(buf, cp));
}
bool string::endsWithInternal(const char* str, u32 str_len) const {
    u32 len = length();
    if (len < str_len) {
		return false;
	}

	u32 start = len - str_len;
	return memcmp(data()+start, str, str_len) == 0;
}
bool string::endsWith(char ch) const {
    return last() == ch;
}
bool string::endsWith(char32_t cp) const {
    char buf[5];
    return endsWithInternal(buf, cp2utf8(buf, cp));
}