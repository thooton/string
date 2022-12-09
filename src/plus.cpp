#include "string.hpp"
#include <stdio.h>
#include <string.h>
#include <inttypes.h>

#define I2S_BUFLEN 32
static int i2s(char* buf, int64_t val) {
	int slen = sprintf(buf, "%" PRId64, val);
	return slen;
}
#define U2S_BUFLEN I2S_BUFLEN
static int u2s(char* buf, uint64_t val) {
	int slen = sprintf(buf, "%" PRIu64, val);
	return slen;
}
#define D2S_BUFLEN 128
static int d2s(char* buf, double val) {
	int slen = snprintf(buf, D2S_BUFLEN, "%g", val);
	if (slen >= (int)sizeof(buf)) {
		slen = sizeof(buf) - 1;
		buf[slen] = 0;
	}
	return slen;
}

string string::operator+(int64_t val) const {
	char buf[I2S_BUFLEN];
	return string(
        data(), buf, 
        length(), i2s(buf, val)
    );
}
string string::operator+(uint64_t val) const {
	char buf[U2S_BUFLEN];
	return string(
        data(), buf, 
        length(), u2s(buf, val)
    );
}
string string::operator+(int32_t val) const {
	return this->operator+((int64_t)val);
}
string string::operator+(uint32_t val) const {
	return this->operator+((uint64_t)val);
}
string string::operator+(int16_t val) const {
	return this->operator+((int64_t)val);
}
string string::operator+(uint16_t val) const {
	return this->operator+((uint64_t)val);
}
string string::operator+(int8_t val) const {
	return this->operator+((int64_t)val);
}
string string::operator+(uint8_t val) const {
	return this->operator+((uint64_t)val);
}
string string::operator+(double val) const {
	char buf[D2S_BUFLEN];
	return string(
        data(), buf, 
        length(), d2s(buf, val)
    );
}
string string::operator+(float val) const {
	return string::operator+((double)val);
}
string string::operator+(char val) const {
    return string(
        data(), &val, 
        length(), 1
    );
}

string string::operator+(char32_t cp) const {
    char buf[5];
    return string(
        data(), buf, 
        length(), cp2utf8(buf, cp)
    );
}

string string::operator+(bool val) const {
    if (val) {
        return string::operator+("true");
    } else {
        return string::operator+("false");
    }
}

void string::operator+=(int64_t val) {
	if (isSingleton()) {
		char buf[I2S_BUFLEN];
		pushSingleton(buf, i2s(buf, val));
	} else {
		*this = *this + val;
	}
}
void string::operator+=(uint64_t val) {
	if (isSingleton()) {
		char buf[U2S_BUFLEN];
		pushSingleton(buf, u2s(buf, val));
	} else {
		*this = *this + val;
	}
}
void string::operator+=(int32_t val) {
	this->operator+=((int64_t)val);
}
void string::operator+=(uint32_t val) {
	this->operator+=((uint64_t)val);
}
void string::operator+=(int16_t val) {
	this->operator+=((int64_t)val);
}
void string::operator+=(uint16_t val) {
	this->operator+=((uint64_t)val);
}
void string::operator+=(int8_t val) {
	this->operator+=((int64_t)val);
}
void string::operator+=(uint8_t val) {
	this->operator+=((uint64_t)val);
}

void string::operator+=(double val) {
	if (isSingleton()) {
		char buf[D2S_BUFLEN];
		pushSingleton(buf, d2s(buf, val));
	} else {
		*this = *this + val;
	}
}
void string::operator+=(float val) {
	return string::operator+=((double)val);
}
void string::operator+=(char val) {
    if (isSingleton()) {
        pushSingletonChar(val);
    } else {
        *this = *this + val;
    }
}
void string::operator+=(char32_t val) {
    if (isSingleton()) {
        char buf[5];
        pushSingleton(buf, cp2utf8(buf, val));
    } else {
        *this = *this + val;
    }
}
void string::operator+=(bool val) {
    if (isSingleton()) {
        if (val) {
            pushSingleton("true", 4);
        } else {
            pushSingleton("false", 5);
        }
    } else {
        *this = *this + val;
    }
}

string operator+(int8_t a, const string& b) {
    string s = "";
    s += a;
    s += b;
    return s;
}
string operator+(uint8_t a, const string& b) {
    string s = "";
    s += a;
    s += b;
    return s;
}
string operator+(int16_t a, const string& b) {
    string s = "";
    s += a;
    s += b;
    return s;
}
string operator+(uint16_t a, const string& b) {
    string s = "";
    s += a;
    s += b;
    return s;
}
string operator+(int32_t a, const string& b) {
    string s = "";
    s += a;
    s += b;
    return s;
}
string operator+(uint32_t a, const string& b) {
    string s = "";
    s += a;
    s += b;
    return s;
}
string operator+(int64_t a, const string& b) {
    string s = "";
    s += a;
    s += b;
    return s;
}
string operator+(uint64_t a, const string& b) {
    string s = "";
    s += a;
    s += b;
    return s;
}
string operator+(float a, const string& b) {
    string s = "";
    s += a;
    s += b;
    return s;
}
string operator+(double a, const string& b) {
    string s = "";
    s += a;
    s += b;
    return s;
}
string operator+(char a, const string& b) {
    string s = "";
    s += a;
    s += b;
    return s;
}
string operator+(char32_t a, const string& b) {
    string s = "";
    s += a;
    s += b;
    return s;
}
string operator+(bool a, const string& b) {
    string s = "";
    s += a;
    s += b;
    return s;
}