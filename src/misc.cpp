#include "string.hpp"

typedef uint32_t u32;

string string::padLeft(u32 max_len, char fill) const {
    u32 len = length();
    if (len >= max_len) {
        return *this;
    }
    return pad(max_len - len, 0, fill);
}
string string::padRight(u32 max_len, char fill) const {
    u32 len = length();
    if (len >= max_len) {
        return *this;
    }
    return pad(0, max_len - len, fill);
}

int64_t string::parseInt() {
	return std::strtoll(str(), nullptr, 0);
}
float string::parseFloat() {
	return std::strtof(str(), nullptr);
}
double string::parseDouble() {
	return std::strtod(str(), nullptr);
}
char* string::toCharArray() const {
    char* str = data();
    u32 len = length();
    char* res = (char*)malloc(len+1);
    memcpy(res, str, len);
    res[len] = '\0';
    return res;
}
std::string string::toStl() const {
    return std::string(data(), length());
}
std::vector<char> string::toVec() const {
    std::vector<char> vec;
    u32 slen = length();
    vec.reserve(slen);
    memcpy(vec.data(), data(), slen);
    return vec;
}

std::ostream& operator<<(std::ostream& stream, const string& s) {
    stream.write(s.data(), s.length());
    return stream;
}