#ifndef STRING_H_TGE94F
#define STRING_H_TGE94F

#include <atomic>
#include <stdint.h>
#include <ostream>
#include <vector>

class string {
private:
	std::atomic<uint32_t>* ref_count;
	// consider SSO via union with char[16]
	char* data;
	uint32_t len, cap;
private:
	string(const string* src, uint32_t start, uint32_t end);
	string(const char* one, const char* two, uint32_t one_len, uint32_t two_len);
	string(uint32_t uninit_len);
	void debug(const char* name) const;
	void incref();
	void decref();
	inline uint32_t refcnt() const {
		if (!ref_count) {
			return 1;
		}
		return ref_count->load(std::memory_order_relaxed);
	}
	void copyFrom(const string& other);
	void moveFrom(string&& other);
	inline bool isSingleton() const {
		return refcnt() == 1;
	}
	inline string copy() const {
		return string(data, len-1);
	}
	void ensureSpaceFor(uint32_t more);
	void pushSingleton(const char* data, uint32_t len);
	void pushSingletonChar(int val);
	string replaceInternalCharStr(int from, const char* to, uint32_t to_len) const;
	string replaceInternalStrStr(const char* from, const char* to, uint32_t from_slen, uint32_t to_len) const;
	string replaceInternalStrChar(const char* from, int to, uint32_t from_slen) const;
	int compareToInternal(const char* other, uint32_t other_slen) const;
	static string joinInternal(const std::vector<string>& vals, const char* sep, uint32_t sep_len);
public:
	string(const char* str);
	string(const char* str, uint32_t slen);
	string(const string& other);
	string& operator=(const string& other);
	string(string&& other);
	string& operator=(string&& other);
	~string();

	inline char operator[](uint32_t idx) const {
		return data[idx];
	}

	string operator+(const string& other) const;
	string operator+(const char* str) const;
	string operator+(int64_t val) const;
	string operator+(uint64_t val) const;
	string operator+(int32_t val) const;
	string operator+(uint32_t val) const;
	string operator+(int16_t val) const;
	string operator+(uint16_t val) const;
	string operator+(int8_t val) const;
	string operator+(uint8_t val) const;
	string operator+(float val) const;
	string operator+(double val) const;

	void operator+=(const string& other);
	void operator+=(const char* str);
	void operator+=(int64_t val);
	void operator+=(uint64_t val);
	void operator+=(int32_t val);
	void operator+=(uint32_t val);
	void operator+=(int16_t val);
	void operator+=(uint16_t val);
	void operator+=(int8_t val);
	void operator+=(uint8_t val);
	void operator+=(float val);
	void operator+=(double val);

	int compareTo(const char* str) const;
	int compareTo(const string& other) const;
	int compareTo(int ch) const;

	friend inline bool operator==(const string& a, const char* b) {
		return a.compareTo(b) == 0;
	}
	friend inline bool operator==(const char* a, const string& b) {
		return b.compareTo(a) == 0;
	}
	friend inline bool operator==(const string& a, const string& b) {
		return a.compareTo(b) == 0;
	}

	friend inline bool operator!=(const string& a, const char* b) {
		return a.compareTo(b) != 0;
	}
	friend inline bool operator!=(const char* a, const string& b) {
		return b.compareTo(a) != 0;
	}
	friend inline bool operator!=(const string& a, const string& b) {
		return a.compareTo(b) != 0;
	}

	// unoptimized
	friend inline string operator+(const char* a, const string& b) {
		string res = a;
		res += b;
		return res;
	}
	friend inline string operator+(int8_t a, const string& b) {
		string res = "";
		res += a;
		res += b;
		return res;
	}
	friend inline string operator+(uint8_t a, const string& b) {
		string res = "";
		res += a;
		res += b;
		return res;
	}
	friend inline string operator+(int16_t a, const string& b) {
		string res = "";
		res += a;
		res += b;
		return res;
	}
	friend inline string operator+(uint16_t a, const string& b) {
		string res = "";
		res += a;
		res += b;
		return res;
	}
	friend inline string operator+(int32_t a, const string& b) {
		string res = "";
		res += a;
		res += b;
		return res;
	}
	friend inline string operator+(uint32_t a, const string& b) {
		string res = "";
		res += a;
		res += b;
		return res;
	}
	friend inline string operator+(int64_t a, const string& b) {
		string res = "";
		res += a;
		res += b;
		return res;
	}
	friend inline string operator+(uint64_t a, const string& b) {
		string res = "";
		res += a;
		res += b;
		return res;
	}
	friend inline string operator+(float a, const string& b) {
		string res = "";
		res += a;
		res += b;
		return res;
	}
	friend inline string operator+(double a, const string& b) {
		string res = "";
		res += a;
		res += b;
		return res;
	}

	const char* cstr();

	inline string drop(uint32_t n) const {
		if (n >= length()) {
			return "";
		}
		return substring(n, length());
	}

	bool endsWith(const char* str) const;
	bool endsWith(const string& other) const;
	bool endsWith(int ch) const;

	bool equalsIgnoreCase(const char* str) const;
	bool equalsIgnoreCase(const string& other) const;

	template <typename F> inline string filter(F f) const {
		string result = "";
		uint32_t my_slen = length();
		for (uint32_t i = 0; i < my_slen; i++) {
			int ch = (*this)[i];
			if (f(ch)) {
				result.pushSingletonChar(ch);
			}
		}
		return result;
	}
	template <typename F> inline string filterWithIndex(F f) const {
		string result = "";
		uint32_t my_slen = length();
		for (uint32_t i = 0; i < my_slen; i++) {
			int ch = (*this)[i];
			if (f(ch, i)) {
				result.pushSingletonChar(ch);
			}
		}
		return result;
	}

	template <typename F> inline void forEach(F f) const {
		uint32_t my_len = length();
		for (uint32_t i = 0; i < my_len; i++) {
			f((*this)[i]);
		}
	}
	template <typename F> inline void forEachWithIndex(F f) const {
		uint32_t my_len = length();
		for (uint32_t i = 0; i < my_len; i++) {
			f((*this)[i], i);
		}
	}

	uint32_t hashCode() const;

	inline int head() const {
		return (*this)[0];
	}

	inline bool isEmpty() const {
		return len == 1;
	}

	bool includes(const char* str) const;
	bool includes(const string& other) const;
	bool includes(int ch) const;

	int32_t indexOf(const char* str) const;
	int32_t indexOf(const string& other) const;
	int32_t indexOf(int ch) const;

	int32_t indexOfNot(int ch) const;

	inline string init() {
		if (isEmpty()) {
			return *this;
		}
		return substring(0, length()-1);
	}

	static string join(const std::vector<string>& vals, const char* sep);
	static string join(const std::vector<string>& vals, const string& sep);
	static string join(const std::vector<string>& vals, int sep);

	int32_t lastIndexOf(const char* str) const;
	int32_t lastIndexOf(const string& other) const;
	int32_t lastIndexOf(int ch) const;

	int32_t lastIndexOfNot(int ch) const;

	inline int last() const {
		if (isEmpty()) {
			return 0;
		}
		return (*this)[length()-1];
	}

	inline uint32_t length() const {
		return len-1;
	}

	template <typename F> inline string map(F f) const {
		string result(len);
		char* res_data = result.data;
		uint32_t my_len = length();
		for (uint32_t i = 0; i < my_len; i++) {
			res_data[i] = f((*this)[i]);
		}
		res_data[my_len] = 0;
		return result;
	}

	template <typename F> inline string mapWithIndex(F f) const {
		string result(len);
		char* res_data = result.data;
		uint32_t my_len = length();
		for (uint32_t i = 0; i < my_len; i++) {
			res_data[i] = f((*this)[i], i);
		}
		res_data[my_len] = 0;
		return result;
	}

	string padLeft(int ch, uint32_t max_slen) const;
	string padRight(int ch, uint32_t max_slen) const;

	int64_t parseInt();
	float parseFloat();
	double parseDouble();

	string pluralize() const;

	template<typename F, typename T> inline T reduce(T initial, F f) const {
		T result = initial;
		uint32_t my_len = length();
		for (uint32_t i = 0; i < my_len; i++) {
			result = f(result, (*this)[i]);
		}
		return result;
	}
	template<typename F, typename T> inline T reduceRight(T initial, F f) const {
		T result = initial;
		for (uint32_t i = length();; i--) {
			result = f(result, (*this)[i]);
			if (i == 0) {
				break;
			}
		}
		return result;
	}

	string repeat(int times) const;

	string replace(const char* from, const char* to) const;
	string replace(const char* from, const string& to) const;
	string replace(const char* from, int to) const;
	string replace(const string& from, const char* to) const;
	string replace(const string& from, const string& to) const;
	string replace(const string& from, int to) const;
	string replace(int from, const char* to) const;
	string replace(int from, const string& to) const;
	string replace(int from, int to) const;

	string reverse() const;

	bool startsWith(const char* str) const;
	bool startsWith(const string& other) const;
	bool startsWith(int ch) const;

	std::vector<string> split(const char* sep) const;
	std::vector<string> split(const string& sep) const;
	std::vector<string> split(int sep) const;

	string substring(uint32_t start) const;
	string substring(uint32_t start, uint32_t end) const;

	inline string tail() const {
		if (isEmpty()) {
			return *this;
		}
		return substring(1);
	}

	inline string take(uint32_t n) const {
		if (n >= length()) {
			return *this;
		}
		return substring(0, n);
	}

	char* toCharArray() const;

	string toUpperCase() const;
	string toTitleCase() const;
	string toLowerCase() const;

	string trim() const;
	string trimLeft() const;
	string trimRight() const;
	// string trim(string cutset);
	// string trimLeft(string cutset);
	// string trimRight(string cutset);

	friend inline std::ostream& operator<<(std::ostream& stream, const string& s) {
		stream.write(s.data, s.length());
		return stream;
	}

	void unitTest();

	string withoutSubstring(uint32_t start, uint32_t end) const;
};


typedef string __unambiguous_string;
namespace std {
  template <>
  struct hash<__unambiguous_string> {
	inline std::size_t operator()(const __unambiguous_string& s) const {
		return s.hashCode();
	}
  };
}

#endif /* STRING_H_TGE94F */
