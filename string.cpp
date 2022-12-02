#include "string.h"
#include <string.h>
#include <stdio.h>
#include <inttypes.h>
#include <stdlib.h>

using std::vector;
typedef std::atomic<uint32_t> refc_t;

#define EMPTY_STR ((char*)"");

string::string(const char* str, uint32_t slen) {
	if (!slen) {
		this->data = EMPTY_STR;
		this->ref_count = nullptr;
		this->cap = 1;
		this->len = 1;
		return;
	}

	uint32_t len = slen+1;
	void* memory = malloc(sizeof(refc_t) + len);
	refc_t* ref_count = new(memory) refc_t(1);
	char* data = (char*)&ref_count[1];
	memcpy(data, str, slen);
	data[slen] = 0;

	this->data = data;
	this->len = len;
	this->cap = len;
	this->ref_count = ref_count;
}

string::string(uint32_t uninit_len) {
	void* memory = malloc(sizeof(refc_t) + uninit_len);
	refc_t* ref_count = new(memory) refc_t(1);
	this->data = (char*)&ref_count[1];
	this->len = uninit_len;
	this->cap = uninit_len;
	this->ref_count = ref_count;
}

string::string(const char* str)
	: string(str, strlen(str)) {}

string::string(const string* src, uint32_t start, uint32_t slen) {
	this->data = src->data + start;
	this->len = slen+1;
	this->cap = src->cap - start;
	this->ref_count = src->ref_count;
	incref();
}

string::string(const char* one, const char* two, uint32_t one_slen, uint32_t two_len) {
	uint32_t len = one_slen + two_len;
	void* memory = malloc(sizeof(refc_t) + len);
	refc_t* ref_count = new(memory) refc_t(1);
	char* data = (char*)&ref_count[1];
	memcpy(data, one, one_slen);
	memcpy(data+one_slen, two, two_len);
	this->data = data;
	this->len = len;
	this->cap = len;
	this->ref_count = ref_count;
}

void string::copyFrom(const string& other) {
	data = other.data;
	len = other.len;
	cap = other.cap;
	ref_count = other.ref_count;
	incref();
}

string::string(const string& other) {
	copyFrom(other);
}

string& string::operator=(const string& other) {
	decref();
	copyFrom(other);
	return *this;
}

void string::moveFrom(string&& other) {
	data = other.data;
	len = other.len;
	cap = other.cap;
	ref_count = other.ref_count;
	other.ref_count = nullptr;
	other.len = 1;
	other.cap = 1;
	other.data = EMPTY_STR;
}

string::string(string&& other) {
	moveFrom(std::move(other));
}

string& string::operator=(string&& other) {
	decref();
	moveFrom(std::move(other));
	return *this;
}

void string::incref() {
	if (!ref_count) {
		return;
	}
	ref_count->fetch_add(1, std::memory_order_relaxed);
}

void string::decref() {
	if (!ref_count) {
		return;
	}

	/* If thread A writes to the string data (perhaps appending),
	 * gives a copy of the string to thread B, then
	 * thread A drops the string, then thread B drops
	 * the string, deallocating the memory, the writes
	 * by thread A may not have been synchronized across
	 * all threads. So, if thread C reallocates the same
	 * memory and begins using it for some other purpose,
	 * thread A's writes may suddenly synchronize,
	 * corrupting thread C's memory.
	 *
	 * Thus, we must flush any writes to the string when
	 * we drop it.
	 *
	 * Another potential situation is accessing string[0]
	 * for example, and then destructing the string.
	 * If the string destructor is reordered before the
	 * read access, another thread may deallocate the memory,
	 * another thread may recycle it for another purpose,
	 * and then string[0] would return invalid data.
	 * So, we must also ensure all reads occur before the
	 * ref_count is decremented.
	 */

	/* "the release operation: no reads or writes in the
	 * current thread can be reordered after this store"
	 * https://en.cppreference.com/w/cpp/atomic/memory_order
	 */
	if (ref_count->fetch_sub(
		1, std::memory_order_release
	) != 1) {
		return;
	}

	/* "the acquire operation...All writes in other threads
	 * that release the same atomic variable are visible
	 * in the current thread"
	 * https://en.cppreference.com/w/cpp/atomic/memory_order
	 */
	ref_count->load(std::memory_order_acquire);

	/* now free() makes the writes visible on all threads
	 * (I think) anyway,
	 * gecko https://searchfox.org/mozilla-central/rev/ae8c2e2354db
	 * 652950fe0ec16983360c21857f2a/xpcom/base/nsISupportsImpl.h#337
	 * uses this pattern and it seems to work for them
	 */
	free(ref_count);
}

string::~string() {
	decref();
}

string string::operator+(const string& other) const {
	return string(data, other.data, len-1, other.len);
}
string string::operator+(const char* str) const {
	return string(data, str, len-1, strlen(str) + 1);
}

#define I2S_BUFLEN 32
static int i2s(char* buf, int64_t val) {
	int slen = sprintf(buf, "%" PRId64, val);
	return slen+1;
}
#define U2S_BUFLEN I2S_BUFLEN
static int u2s(char* buf, uint64_t val) {
	int slen = sprintf(buf, "%" PRIu64, val);
	return slen+1;
}
#define D2S_BUFLEN 128
static int d2s(char* buf, double val) {
	int slen = snprintf(buf, D2S_BUFLEN, "%g", val);
	if (slen >= (int)sizeof(buf)) {
		slen = sizeof(buf) - 1;
		buf[slen] = 0;
	}
	return slen+1;
}

string string::operator+(int64_t val) const {
	char buf[I2S_BUFLEN];
	return string(data, buf, len-1, i2s(buf, val));
}
string string::operator+(uint64_t val) const {
	char buf[U2S_BUFLEN];
	return string(data, buf, len-1, u2s(buf, val));
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
	return string(data, buf, len-1, d2s(buf, val));
}
string string::operator+(float val) const {
	return string::operator+((double)val);
}

inline static uint32_t nextHighestPowerOfTwo(uint32_t v) {
	v--;
	v |= v >> 1;
	v |= v >> 2;
	v |= v >> 4;
	v |= v >> 8;
	v |= v >> 16;
	v++;
	return v;
}

void string::ensureSpaceFor(uint32_t more) {
	uint32_t needed_cap = this->len + more;
	if (needed_cap > this->cap) {
		uint32_t new_cap = nextHighestPowerOfTwo(needed_cap);
		void* memory = malloc(sizeof(refc_t) + new_cap);
		refc_t* new_ref_count = new(memory) refc_t(1);
		char* new_data = (char*)&new_ref_count[1];
		memcpy(new_data, this->data, this->len);
		free(this->ref_count);
		this->cap = new_cap;
		this->data = new_data;
		this->ref_count = new_ref_count;
	}
}
void string::pushSingleton(const char* data, uint32_t data_len) {
	ensureSpaceFor(data_len-1);
	memcpy(this->data+this->len-1, data, data_len);
	this->len += data_len-1;
}
void string::pushSingletonChar(int val) {
	ensureSpaceFor(1);
	this->data[this->len-1] = val;
	this->data[this->len] = 0;
	++this->len;
}

void string::operator+=(const string& other) {
	if (isSingleton()) {
		pushSingleton(other.data, other.len);
	} else {
		// move-assignment
		*this = *this + other;
	}
}
void string::operator+=(const char* str) {
	if (isSingleton()) {
		pushSingleton(str, strlen(str)+1);
	} else {
		*this = *this + str;
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

inline static uint32_t min(uint32_t a, uint32_t b) {
	if (a < b) {
		return a;
	}

	return b;
}

int string::compareToInternal(const char* other, uint32_t other_slen) const {
	uint32_t slen = len-1;
	int res = memcmp(data, other, min(slen, other_slen));
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
int string::compareTo(const char* str) const {
	return compareToInternal(str, strlen(str));
}
int string::compareTo(const string& other) const {
	return compareToInternal(other.data, other.len-1);
}
int string::compareTo(int ch) const {
	return data[0] - ch;
}

const char* string::cstr() {
	if (data[len-1] != 0) {
		if (isSingleton()) {
			data[len-1] = 0;
		} else {
			*this = copy();
		}
	}

	return data;
}

bool string::endsWith(const string& other) const {
	if (len < other.len) {
		return false;
	}

	uint32_t start = len - other.len;
	return memcmp(data+start, other.data, other.len) == 0;
}
bool string::endsWith(const char* str) const {
	uint32_t other_len = strlen(str);
	if (len < other_len) {
		return false;
	}

	uint32_t start = len - other_len;
	return memcmp(data+start, str, other_len) == 0;
}

bool string::endsWith(int ch) const {
	if (len < 2) {
		return false;
	}
	return data[len-2] == ch;
}

static bool memEqualsIgnoreCase(
	const char* one, const char* two, uint32_t one_slen, uint32_t two_slen
) {
	if (one_slen != two_slen) {
		return false;
	}
	for (uint32_t i = 0; i < one_slen; i++) {
		int och = one[i];
		int tch = two[i];
		if (och >= 'A' && och <= 'Z') {
			och += 32;
		}
		if (tch >= 'A' && tch <= 'Z') {
			tch += 32;
		}
		if (och != tch) {
			return false;
		}
	}
	return true;
}
bool string::equalsIgnoreCase(const char* str) const {
	return memEqualsIgnoreCase(data, str, len-1, strlen(str));
}
bool string::equalsIgnoreCase(const string& other) const {
	return memEqualsIgnoreCase(data, other.data, len-1, other.len-1);
}

inline static uint32_t powU32(uint32_t a, uint32_t b) {
   uint32_t p = 1;
   while (b) {
	   if ((b&1) != 0) {
		   p *= a;
	   }
	   b >>= 1;
	   a *= a;
   }
   return p;
}

uint32_t string::hashCode() const {
	uint32_t n = length();
	uint32_t pwr = n-1;
	uint32_t hash = 0;
	for (size_t i = 0; i < n; i++, pwr--) {
		hash += (*this)[i] * powU32(31, pwr);
	}
	return hash;
}

// unoptimized
static const char* memmem(const char* haystack, const char* needle, uint32_t hsize, uint32_t nsize) {
	if (hsize < nsize) {
		return nullptr;
	}

	const char* endp = haystack+hsize-nsize;
	for (const char* p = haystack; p <= endp; ++p) {
		if (memcmp(p, needle, nsize) == 0) {
			return p;
		}
	}
	return nullptr;
}
static const char* memstr(const char* haystack, const char* needle, uint32_t hsize) {
	uint32_t nsize = strlen(needle);
	return memmem(haystack, needle, hsize, nsize);
}

int32_t string::indexOf(const char* str) const {
	const char* res = memstr(data, str, len);
	if (!res) {
		return -1;
	}
	return res - data;
}
int32_t string::indexOf(const string& other) const {
	const char* res = memmem(data, other.data, len, other.len);
	if (!res) {
		return -1;
	}
	return res - data;
}
int32_t string::indexOf(int ch) const {
	const char* res = (const char*)memchr(data, ch, len);
	if (!res) {
		return -1;
	}
	return res - data;
}

int32_t string::indexOfNot(int ch) const {
	int32_t slen = length();
	for (int32_t i = 0; i < slen; i++) {
		if ((*this)[i] != ch) {
			return i;
		}
	}
	return -1;
}

bool string::includes(const char* str) const {
	return memstr(data, str, len) != nullptr;
}
bool string::includes(const string& other) const {
	return memmem(data, other.data, len, other.len) != nullptr;
}
bool string::includes(int ch) const {
	return memchr(data, ch, len) != nullptr;
}

// static
string string::joinInternal(
	const std::vector<string>& vals, const char* sep, uint32_t sep_len
) {
	string result = "";
	const string* data = &vals[0];
	uint32_t data_len = vals.size();
	if (data_len) {
		result.pushSingleton(data[0].data, data[0].len);
	}
	for (uint32_t i = 1; i < data_len; i++) {
		result.pushSingleton(sep, sep_len);
		result.pushSingleton(data[i].data, data[i].len);
	}
	result.data[result.length()] = 0;
	return result;
}
// static
string string::join(const std::vector<string>& vals, const char* sep) {
	return string::joinInternal(vals, sep, strlen(sep)+1);
}
// static
string string::join(const std::vector<string>& vals, const string& sep) {
	return string::joinInternal(vals, sep.data, sep.len);
}
// static
string string::join(const std::vector<string>& vals, int sep) {
	string result = "";
	const string* data = &vals[0];
	uint32_t data_len = vals.size();
	if (data_len) {
		result.pushSingleton(data[0].data, data[0].len);
	}
	for (uint32_t i = 1; i < data_len; i++) {
		result.pushSingletonChar(sep);
		result.pushSingleton(data[i].data, data[i].len);
	}
	result.data[result.length()] = 0;
	return result;
}

// unoptimized
static const char* memlmem(const char* haystack, const char* needle, uint32_t hsize, uint32_t nsize) {
	if (hsize < nsize) {
		return nullptr;
	}

	for (const char* p = haystack+hsize-nsize; p >= haystack; --p) {
		if (memcmp(p, needle, nsize) == 0) {
			return p;
		}
	}
	return nullptr;
}
static const char* memlstr(const char* haystack, const char* needle, uint32_t hsize) {
	uint32_t nsize = strlen(needle);
	return memlmem(haystack, needle, hsize, nsize);
}
static const char* memlchr(const char* haystack, int needle, uint32_t hsize) {
	for (const char* p = haystack+hsize-1; p >= haystack; --p) {
		if (*p == needle) {
			return p;
		}
	}
	return nullptr;
}

int32_t string::lastIndexOf(const char* str) const {
	const char* res = memlstr(data, str, len);
	if (!res) {
		return -1;
	}
	return res - data;
}
int32_t string::lastIndexOf(const string& other) const {
	const char* res = memlmem(data, other.data, len, other.len);
	if (!res) {
		return -1;
	}
	return res - data;
}
int32_t string::lastIndexOf(int ch) const {
	const char* res = memlchr(data, ch, len);
	if (!res) {
		return -1;
	}
	return res - data;
}

int32_t string::lastIndexOfNot(int ch) const {
	for (int32_t i = length()-1; i >= 0; i--) {
		if ((*this)[i] != ch) {
			return i;
		}
	}
	return -1;
}

// unoptimized
string string::padLeft(int ch, uint32_t max_slen) const {
	if (length() >= max_slen) {
		return *this;
	}
	uint32_t needed = length() - max_slen;
	string result = "";
	while (needed) {
		result.pushSingletonChar(ch);
		needed--;
	}
	result.pushSingleton(data, len);
	return result;
}
string string::padRight(int ch, uint32_t max_slen) const {
	if (length() >= max_slen) {
		return *this;
	}
	uint32_t needed = length() - max_slen;
	string result = copy();
	while (needed) {
		result.pushSingletonChar(ch);
		needed--;
	}
	return result;
}

// unoptimized
int64_t string::parseInt() {
	cstr();
	return std::strtoll(data, nullptr, 0);
}
float string::parseFloat() {
	cstr();
	return std::strtof(data, nullptr);
}
double string::parseDouble() {
	cstr();
	return std::strtod(data, nullptr);
}

// unoptimized
/*    /adm/simul_efun/english.c
 *    from Dead Souls
 *    efuns for dealing with the oddity we know as the English language
 *    created by Descartes of Borg 940207
 *    Version: @(#) english.c 1.5@(#)
 *    Last modified: 97/01/01
 */
string string::pluralize() const {
	if (isEmpty()) {
		return *this;
	}

	string lower = toLowerCase();
	bool isLower = !((*this)[0] >= 'A' && (*this)[0] <= 'Z');
	if (lower == "moose" || lower == "sheep" || lower == "fish" || lower == "deer") {
		return *this;
	} else if (lower == "mouse") {
		return isLower ? "mice" : "Mice";
	} else if (lower == "die") {
		return isLower ? "dice" : "Dice";
	} else if (lower == "index") {
		return isLower ? "indices" : "Indices";
	} else if (lower == "human") {
		return isLower ? "humans" : "Humans";
	} else if (lower == "child") {
		return isLower ? "children" : "Children";
	} else if (lower == "ox") {
		return isLower ? "oxen" : "Oxen";
	} else if (lower == "tooth") {
		return isLower ? "teeth" : "Teeth";
	} else if (lower == "sphinx") {
		return isLower ? "sphinges" : "Sphinges";
	}

	string ending = lower.substring(length() - 2);
	if (ending == "ch" || ending == "sh") {
		return *this + "es";
	} else if (ending == "ff" || ending == "fe") {
		return substring(0, length() - 3) + "ves";
	} else if (ending == "us") {
		return substring(0, length() - 3) + "i";
	} else if (ending == "um") {
		return substring(0, length() - 3) + "a";
	} else if (ending == "ef") {
		return *this + "s";
	}

	char last = lower[lower.length() - 1];
	switch (last) {
	case 'o':
	case 'x':
	case 's':
		return *this + "es";
	case 'f':
		return *this + "ves";
	case 'y': {
		char second_last = lower[length() - 2];
		switch (second_last) {
		case 'a':
		case 'e':
		case 'i':
		case 'o':
		case 'u':
			return *this + "s";
		default:
			return *this + "ies";
		}
	}
	default:
		return *this + "s";
	}
}

string string::repeat(int times) const {
	if (times == 1) {
		return *this;
	}
	const char* data = this->data;
	uint32_t len = this->len;
	string result = "";
	for (int i = 0; i < times; i++) {
		result.pushSingleton(data, len);
	}
	return result;
}

// unoptimized
string string::replaceInternalStrStr(
	const char* from, const char* to, uint32_t from_slen, uint32_t to_len
) const {
	uint32_t slen = this->len-1;
	if (slen < from_slen) {
		return *this;
	}
	string result = "";
	char* data = this->data;
	uint32_t end = slen - from_slen;
	for (uint32_t i = 0; i <= end;) {
		if (memcmp(&data[i], from, from_slen) == 0) {
			result.pushSingleton(to, to_len);
			i += from_slen;
		} else {
			result.pushSingletonChar(data[i]);
			i++;
		}
	}
	return result;
}
string string::replace(const char* from, const char* to) const {
	return replaceInternalStrStr(from, to, strlen(from), strlen(to)+1);
}
string string::replace(const char* from, const string& to) const {
	return replaceInternalStrStr(from, to.data, strlen(from), to.len);
}
string string::replace(const string& from, const char* to) const {
	return replaceInternalStrStr(from.data, to, from.len-1, strlen(to)+1);
}
string string::replace(const string& from, const string& to) const {
	return replaceInternalStrStr(from.data, to.data, from.len-1, to.len);
}

string string::replaceInternalStrChar(
	const char* from, int to, uint32_t from_slen
) const {
	uint32_t slen = this->len-1;
	if (slen < from_slen) {
		return *this;
	}
	string result = "";
	char* data = this->data;
	uint32_t end = slen - from_slen;
	for (uint32_t i = 0; i <= end;) {
		if (memcmp(&data[i], from, from_slen) == 0) {
			result.pushSingletonChar(to);
			i += from_slen;
		} else {
			result.pushSingletonChar(data[i]);
			i++;
		}
	}
	return result;
}
string string::replace(const char* from, int to) const {
	return replaceInternalStrChar(from, to, strlen(from));
}
string string::replace(const string& from, int to) const {
	return replaceInternalStrChar(from.data, to, from.len-1);
}

string string::replaceInternalCharStr(int from, const char* to, uint32_t to_len) const {
	string result = "";
	char* data = this->data;
	uint32_t slen = this->len-1;
	for (uint32_t i = 0; i < slen; i++) {
		int ch = data[i];
		if (ch == from) {
			result.pushSingleton(to, to_len);
		} else {
			result.pushSingletonChar(ch);
		}
	}
	return result;
}
string string::replace(int from, const char* to) const {
	return replaceInternalCharStr(from, to, strlen(to)+1);
}
string string::replace(int from, const string& to) const {
	return replaceInternalCharStr(from, to.data, to.len);
}
string string::replace(int from, int to) const {
	string result = copy();
	char* data = result.data;
	uint32_t len = result.len;
	for (uint32_t i = 0; i < len; i++) {
		if (data[i] == from) {
			data[i] = to;
		}
	}
	return result;
}

string string::reverse() const {
	string result(len);
	char* res_data = result.data;
	char* data = this->data;
	for (uint32_t i = 0, j = len-2; i < j; i++, j--) {
		res_data[i] = data[j];
		res_data[j] = data[i];
	}
	data[len-1] = 0;
	return result;
}

////// https://hypjudy.github.io/2017/04/18/KMP-algorithm-split/ //////////////
////////////////////// unoptimized ////////////////////////////////////////////

/* Return a table indicates the length of partial
*  matched string for each substring */
static vector<uint32_t> partial_match_table(const string& delimiter) {
    vector<uint32_t> res;
    res.push_back(0);
    uint32_t i = 1; // iterate over delimiter, the end of prefix
    uint32_t len = 0; // the end of suffix, the length of matched substr
    while (i < delimiter.length()) {
        if (delimiter[i] == delimiter[len]) {
            ++len;
            res.push_back(len);
            ++i;
        }
        else {
            if(len) len = res[len - 1]; // trick
            else {
                ++i;
                res.push_back(0);
            }
        }
    }
    return res;
}

vector<string> string::split(const string& delimiter) const {
	vector<string> res;
	vector<uint32_t> table = partial_match_table(delimiter);
	uint32_t i = 0, j = 0; // point to string and delimiter respectively
	uint32_t last = 0; // to split position
	uint32_t m = this->length(), n = delimiter.length();
	while (i < m) {
	    if ((*this)[i] == delimiter[j]) {
	        ++i;
	        ++j;
	    }
	    else if (j == 0) {
	        ++i;
	    }
	    else { // skip matched string of prefix of sep and suffix of s
	        j = table[j - 1];
	    }
	    if (j == n) { // find a seperator
	        res.push_back(this->substring(last, i - n));
	        last = i;
	        j = 0; // start to match again
	    }
	}
	res.push_back(this->substring(last, i));
	return res;
}

vector<string> string::split(const char* sep) const {
	return split(string(sep));
}

vector<string> string::split(int sep) const {
	char buf[2];
	buf[0] = sep;
	buf[1] = 0;
	return split(buf);
}
////////////////////// end ////////////////////////////////////////////

bool string::startsWith(const char* str) const {
	uint32_t other_len = strlen(str);
	if (len < other_len) {
		return false;
	}
	return memcmp(data, str, other_len) == 0;
}

bool string::startsWith(const string& other) const {
	if (len < other.len) {
		return false;
	}
	return memcmp(data, other.data, other.len) == 0;
}

bool string::startsWith(int ch) const {
	return data[0] == ch;
}

string string::substring(uint32_t start, uint32_t end) const {
	string result(this, start, end-start);
	return result;
}
string string::substring(uint32_t start) const {
	return substring(start, length());
}

char* string::toCharArray() const {
	char* res = (char*)malloc(length()+1);
	memcpy(res, data, length());
	res[length()] = 0;
	return res;
}

string string::toUpperCase() const {
	string result = copy();
	char* data = result.data;
	uint32_t slen = result.length();
	for (uint32_t i = 0; i < slen; i++) {
		int ch = data[i];
		if (ch >= 'a' && ch <= 'z') {
			data[i] = ch - 32;
		}
	}
	return result;
}
string string::toTitleCase() const {
	uint32_t slen = length();
	if (!slen) {
		return "";
	}
	string result = copy();
	char* data = result.data;
	data[0] = toupper(data[0]);
	for (uint32_t i = 1; i < slen; i++) {
		if (data[i-1] == ' ') {
			data[i] = toupper(data[i]);
		}
	}
	return result;
}
string string::toLowerCase() const {
	string result = copy();
	char* data = result.data;
	uint32_t slen = result.length();
	for (uint32_t i = 0; i < slen; i++) {
		int ch = data[i];
		if (ch >= 'A' && ch <= 'Z') {
			data[i] = ch + 32;
		}
	}
	return result;
}

string string::trim() const {
	char* data = this->data;
	uint32_t slen = this->len-1;
	if (slen == 0) {
		return *this;
	}
	uint32_t left;
	for (left = 0; left < slen;) {
		int ch = data[left];
		if (ch <= ' ') {
			left++;
		} else {
			break;
		}
	}
	uint32_t right;
	for (right = slen-1;;) {
		int ch = data[right];
		if (ch <= ' ') {
			if (right == 0) {
				break;
			}
			right--;
		} else {
			break;
		}
	}
	if (right < left) {
		right = left;
	}
	return substring(left, right);
}

string string::trimLeft() const {
	char* data = this->data;
	uint32_t slen = this->len - 1;
	uint32_t left;
	for (left = 0; left < slen;) {
		int ch = data[left];
		if (ch <= ' ') {
			left++;
		} else {
			break;
		}
	}
	return substring(left);
}
string string::trimRight() const {
	char* data = this->data;
	uint32_t slen = this->len - 1;
	if (slen == 0) {
		return *this;
	}
	uint32_t right;
	for (right = slen-1;;) {
		int ch = data[right];
		if (ch <= ' ') {
			if (right == 0) {
				break;
			}
			right--;
		} else {
			break;
		}
	}
	return substring(0, right);
}

void string::debug(const char* name) const {
	printf(   "%s = string{\n", name);
	printf(	"    len: %d\n", len);
	printf( "    cap: %d\n", cap);
	printf( "    data: %p\n", data);
	printf( "    ref_count: %p\n", ref_count);
	printf( "    *ref_count: %d\n", refcnt());
	printf( "    inner_str: %s\n", data);
	printf( "    inner_bytes: [");
	const unsigned char* data = (const unsigned char*)this->data;
	printf("%d", (int)data[0]);
	for (uint32_t i = 1; i < len; i++) {
		printf(", %d", (int)data[i]);
	}
	puts("]");
	puts("}");
}

static void assert(const char* desc, bool condition) {
	if (!condition) {
		printf("condition failed: %s\n", desc);
		exit(1);
	}
}
static void assertEq(const char* desc, uint32_t one, uint32_t two) {
	if (one != two) {
		printf("assertion failed: %s (%d != %d)\n", desc, one, two);
		exit(1);
	}
}
static void assertEq(const char* desc, const char* one, const char* two) {
	uint32_t one_len = strlen(one);
	uint32_t two_len = strlen(two);
	if (one_len != two_len) {
		printf("assertion \"%s\" failed (%s != %s)\n", desc, one, two);
		exit(1);
	}

	for (uint32_t i = 0; i < one_len; i++) {
		if (one[i] != two[i]) {
			printf("assertion \"%s\" failed (%s != %s)\n", desc, one, two);
			exit(1);
		}
	}
}

#include <iostream>

void string::unitTest() {
	puts("test 1: basic substring, cstr reallocation properties");
	{
		string market = "ready";
		string mkt = market.substring(1, 4);
		assertEq("substring slen", mkt.len-1, 3);
		assertEq("ref amt", mkt.refcnt(), 2);
		assertEq("ref amt2", market.refcnt(), 2);
		printf("mkt: %s\n", mkt.cstr());
		assertEq("ref amt3", market.refcnt(), 1);
		assertEq("ref amt4", mkt.refcnt(), 1);
		assertEq("zero termination", market[market.len-1], 0);
		assertEq("zero termination2", market[market.length()], 0);
		const char* mkt_data = mkt.data;
		printf("mkt: %s\n", mkt.cstr());
		assertEq("no reallocation", mkt.data, mkt_data);
	}
	puts("test 2: basic appending and addition");
	{
		string people = "";
		people += string("Helen, ") + 16 + "\n";
		people += 19 + string(" is Joseph\n");
		people += 26;
		people += " is my son\n";
		assertEq("appended correctly", people.cstr(), "Helen, 16\n19 is Joseph\n26 is my son\n");
		string fvalues = "";
		float some_floats[] = {4.9, 5.7, 2.5, 4.3336};
		for (int i = 0; i < 4; i++) {
			fvalues += some_floats[i];
			fvalues += " ";
		}
		assertEq("appended correctly", fvalues.cstr(), "4.9 5.7 2.5 4.3336 ");
	}
	puts("test 3: move assignment");
	{
		string people = "";
		string people2 = people;
		people += string("hello");
		people2 += string("jesus");
	}
	puts("test 4: compareTo");
	{
		string one = "hello";
		string two = "goodbye";
		one.debug("one");
		two.debug("two");
		assertEq("compare one&two", one.compareTo(two), 1);
		one = "among";
		two = "belong";
		one.debug("one");
		two.debug("two");
		assertEq("compare one&two 2", one.compareTo(two), -1);
		one = "testing";
		two = "testing2";
		assertEq("compare one&two 3", one.compareTo(two), -1);
		string one2 = "jesus5";
		string two2 = "jesus";
		one = one2;
		two = two2;
		assertEq("compare one&two 3", one.compareTo(two), 1);
		one = "wowzers";
		two = "wowzers";
		assertEq("compare one&two 4", one.compareTo(two), 0);
		assertEq("compare equal one&two", one == two, true);
	}
	puts("test 5: replacement");
	{
		string manager = "manager";
		string mgr1 = manager.replace('a', 'i');
		string mgr2 = manager.replace('a', "i");
		string mgr3 = manager.replace('a', string("i"));

		string mgr4 = manager.replace("a", 'i');
		string mgr5 = manager.replace("a", "i");
		string mgr6 = manager.replace("a", string("i"));

		string mgr7 = manager.replace(string("a"), 'i');
		string mgr8 = manager.replace(string("a"), "i");
		string mgr9 = manager.replace(string("a"), string("i"));

		assertEq("should be", mgr5.cstr(), "miniger");
		printf("mgr1: %s\n", mgr1.cstr());
		printf("mgr2: %s\n", mgr2.cstr());
		printf("mgr3: %s\n", mgr3.cstr());
		printf("mgr4: %s\n", mgr4.cstr());
		printf("mgr5: %s\n", mgr5.cstr());
		printf("mgr6: %s\n", mgr6.cstr());
		printf("mgr7: %s\n", mgr7.cstr());
		printf("mgr8: %s\n", mgr8.cstr());
		printf("mgr9: %s\n", mgr9.cstr());
		assert("all equality",
			mgr1 == mgr2 && mgr2 == mgr3 && mgr3 == mgr4
			&& mgr4 == mgr5 && mgr5 == mgr6 && mgr6 == mgr7
			&& mgr8 == mgr9);

	}
	puts("test 6: general test");
	string thing = "among us";
	std::cout << thing << "\n";
	std::cout << thing.pluralize() << "\n";
	std::cout << string("box").pluralize() << "\n";
	std::cout << thing.init() << "\n";
	thing = "death of democracy in America";
	std::vector<string> stuff = thing.split(' ');
	std::cout << "alright" << "\n";
	std::cout << "stuff_len = " << stuff.size() << "\n";
	for (string val : stuff) {
		std::cout << "ok, " << val << "\n";
	}
}

string string::withoutSubstring(uint32_t start, uint32_t end) const {
	string result = start ? substring(0, start) : "";
	if (end != length()) {
		string second_part = substring(end);
		result.pushSingleton(second_part.data, second_part.len);
	}
	return result;
}
