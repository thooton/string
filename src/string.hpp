#ifndef STRING_ETPQWR
#define STRING_ETPQWR

#include <stdint.h> // for uint32_t and int32_t
#include <type_traits> // for std::is_*
#include <string.h> // for strlen
#include <ostream> // for std::ostream
#include <string>
#include <vector>

class string {
private:
    struct {
        char* data;
        uint32_t len, cap_info;
    } alloc;
/* core.cpp */
    void setAllocCap(uint32_t);
    uint32_t getAllocCap() const;
    bool allocActive() const;
    bool ssoActive() const;
    bool litActive() const;
    void setSsoLen(int);
    int getSsoLen() const;
    char* data() const;
public:
    uint32_t length() const;
    string();
private:
    string(int32_t);
    string(const char* one, const char* two, 
        uint32_t one_len, uint32_t two_len);
    void incref() const;
    void decref() const;
    uint32_t refcnt() const;
private:
    string(const char*, int32_t, bool);
public:
    string(const char* str, int32_t len)
        : string(str, len, false) {}
    template<int32_t LITLEN> string(const char (&literal)[LITLEN]) 
        : string(literal, LITLEN-1, true) {}
    template<typename T> string(T&& str) 
        : string(str, strlen(str)) {}
private:
    string(const string* src, 
        uint32_t start, uint32_t end);
public:
    ~string();

    string(const string&);
    string& operator=(const string&);
	string(string&&);
	string& operator=(string&&);
#define SSO_CAP (sizeof(alloc)-1)
#define SSO_DATA ((char*)&alloc)
#define SSO_INFO (*(SSO_DATA + SSO_CAP))
#define SSO_ACTIVE (!(SSO_INFO & 1))
    char operator[](uint32_t i) const {
        if (SSO_ACTIVE) {
            return SSO_DATA[i];
        } else {
            return alloc.data[i];
        }
    }
#undef SSO_CAP
#undef SSO_ACTIVE
#undef SSO_INFO
#undef SSO_DATA
    const char* str();
private:
    void ensureSpaceFor(uint32_t);
    void pushSingleton(const char*, uint32_t);
    void pushSingletonChar(int);
    bool isSingleton() const;
    void shrinkNonSubstringToFitLength(uint32_t);
public:
    string substring(uint32_t) const;
	string substring(uint32_t, uint32_t) const;
/* plus.cpp */
    // stolen from tiny_utf8
    template<typename T, typename CharType, typename DataType = bool>
		using enable_if_ptr = typename std::enable_if<
			std::is_pointer<typename std::remove_reference<T>::type>::value
			&&
			std::is_same<
				CharType
				, typename std::remove_cv<
					typename std::remove_pointer<
						typename std::remove_reference<T>::type
					>::type
				>::type
			>::value
			, DataType
		>::type;
    template<typename T> enable_if_ptr<T, char, string> operator+(T&& str) const {
        return string(
            data(), str,
            length(), strlen(str)
        );
    }
    template<int32_t LITLEN> string operator+(const char (&literal)[LITLEN]) const {
        return string(
            data(), literal,
            length(), LITLEN-1
        );
    }
	string operator+(const string& s) const {
        return string(
            data(), s.data(), 
            length(), s.length()
        );
    }
	string operator+(int64_t) const;
	string operator+(uint64_t) const;
	string operator+(int32_t) const;
	string operator+(uint32_t) const;
	string operator+(int16_t) const;
	string operator+(uint16_t) const;
	string operator+(int8_t) const;
	string operator+(uint8_t) const;
	string operator+(float) const;
	string operator+(double) const;
    string operator+(char) const;
    string operator+(char32_t) const;
    string operator+(bool) const;
    
    template<typename T> enable_if_ptr<T, char, void> operator+=(T&& str) {
        if (isSingleton()) {
            pushSingleton(str, strlen(str));
        } else {
            *this = *this + str;
        }
    }
    template<int32_t LITLEN> void operator+=(const char (&literal)[LITLEN]) {
        if (isSingleton()) {
            pushSingleton(literal, LITLEN-1);
        } else {
            *this = *this + literal;
        }
    }
	void operator+=(const string& s) {
        if (isSingleton()) {
            pushSingleton(s.data(), s.length());
        } else {
            *this = *this + s;
        }
    }
	void operator+=(int64_t);
	void operator+=(uint64_t);
	void operator+=(int32_t);
	void operator+=(uint32_t);
	void operator+=(int16_t);
	void operator+=(uint16_t);
	void operator+=(int8_t);
	void operator+=(uint8_t);
	void operator+=(float);
	void operator+=(double);
    void operator+=(char);
    void operator+=(char32_t);
    void operator+=(bool);

    template<typename T> friend enable_if_ptr<T, char, string> operator+(T&& a, const string& b) {
        return string(
            a, b.data(), 
            strlen(a), b.length()
        );
    }
    template<int32_t LITLEN> friend string operator+(const char (&literal)[LITLEN], const string& b) {
        return string(
            literal, b.data(),
            LITLEN, b.length()
        );
    }

	friend string operator+(int8_t a, const string& b);
	friend string operator+(uint8_t a, const string& b);
	friend string operator+(int16_t a, const string& b);
	friend string operator+(uint16_t a, const string& b);
	friend string operator+(int32_t a, const string& b);
	friend string operator+(uint32_t a, const string& b);
	friend string operator+(int64_t a, const string& b);
	friend string operator+(uint64_t a, const string& b);
	friend string operator+(float a, const string& b);
	friend string operator+(double a, const string& b);
    friend string operator+(char a, const string& b);
    friend string operator+(char32_t a, const string& b);
    friend string operator+(bool a, const string& b);
private:
/* compare.cpp */
    int compareInternal(const char*, uint32_t) const;
public:
    template<typename T> enable_if_ptr<T, char, int> compare(T&& str) const {
        return compareInternal(str, strlen(str));
    }
    template<int32_t LITLEN> int compare(const char (&literal)[LITLEN]) const {
        return compareInternal(literal, LITLEN-1);
    }
    int compare(const string& s) const {
        return compareInternal(s.data(), s.length());
    }

    template<int32_t LITLEN> bool operator==(const char (&literal)[LITLEN]) const {
        return length() == LITLEN
            && compare(literal) == 0;
    }
    template<typename T> enable_if_ptr<T, char, bool> operator==(T&& str) const {
        uint32_t str_len = strlen(str);
        if (length() != str_len) {
            return false;
        }
        return compareInternal(str, str_len) == 0;
    }
    bool operator==(const string& s) const {
        return length() == s.length()
            && compare(s) == 0;
    }

    template<int32_t LITLEN> friend bool operator==(const char (&a)[LITLEN], const string& b) {
        return b.length() == LITLEN
            && b.compare(a) == 0;
    }
    template<typename T> friend enable_if_ptr<T, char, bool> operator==(T&& a, const string& b) {
        return b.compare(a) == 0;
    }

    template<int32_t LITLEN> bool operator!=(const char (&literal)[LITLEN]) const {
        return !(*this == literal);
    }
    template<typename T> enable_if_ptr<T, char, bool> operator!=(T&& str) const {
        return !(*this == str);
    }
    bool operator!=(const string& s) const {
        return !(*this == s);
    }

    template<int32_t LITLEN> friend bool operator!=(const char (&a)[LITLEN], const string& b) {
        return !(b == a);
    }
    template<typename T> friend enable_if_ptr<T, char, bool> operator!=(T&& a, const string& b) {
        return !(b == a);
    }
/* haskell.cpp */
    string drop(uint32_t) const;
    char head() const;
    string init() const;
    char last() const;
    string tail() const;
    string take(uint32_t) const;
/* more functionals */
    template <typename F> string filter(F f) const {
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
	template <typename F> string filterWithIndex(F f) const {
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
	template <typename F> void forEach(F f) const {
		uint32_t my_len = length();
		for (uint32_t i = 0; i < my_len; i++) {
			f((*this)[i]);
		}
	}
	template <typename F> void forEachWithIndex(F f) const {
		uint32_t my_len = length();
		for (uint32_t i = 0; i < my_len; i++) {
			f((*this)[i], i);
		}
	}
    template <typename F> string map(F f) const {
		uint32_t len = length();
        string result((int32_t)len);
		char* res_data = result.data();
		for (uint32_t i = 0; i < len; i++) {
			res_data[i] = (char)f((*this)[i]);
		}
		return result;
	}
	template <typename F> string mapWithIndex(F f) const {
        uint32_t len = length();
		string result((int32_t)len);
		char* res_data = result.data();
		for (uint32_t i = 0; i < len; i++) {
			res_data[i] = (char)f((*this)[i], i);
		}
		return result;
	}
    template<typename F, typename T> T reduce(T initial, F f) const {
		T result = initial;
		uint32_t my_len = length();
		for (uint32_t i = 0; i < my_len; i++) {
			result = (T)f(result, (*this)[i]);
		}
		return result;
	}
	template<typename F, typename T> T reduceRight(T initial, F f) const {
		T result = initial;
		for (uint32_t i = length();; i--) {
			result = (T)f(result, (*this)[i]);
			if (i == 0) {
				break;
			}
		}
		return result;
	}
/* with.cpp */
private:
    bool startsWithInternal(const char*, uint32_t) const;
public:
    template<typename T> enable_if_ptr<T, char, bool> startsWith(T&& str) const {
        return startsWithInternal(str, strlen(str));
    }
    template<int32_t LITLEN> bool startsWith(const char (&literal)[LITLEN]) const {
        return startsWithInternal(literal, LITLEN-1);
    }
    bool startsWith(const string& s) const {
        return startsWithInternal(s.data(), s.length());
    }
private:
/* util.cpp */
    static int cp2utf8(char*, char32_t);
public:
/* with.cpp */
    bool startsWith(char) const;
    bool startsWith(char32_t cp) const;
private:
    bool endsWithInternal(const char*, uint32_t) const;
public:
    template<typename T> enable_if_ptr<T, char, bool> endsWith(T&& str) const {
        return endsWithInternal(str, strlen(str));
    }
    template<int32_t LITLEN> bool endsWith(const char (&literal)[LITLEN]) const {
        return endsWithInternal(literal, LITLEN-1);
    }
    bool endsWith(const string& s) const {
        return endsWithInternal(s.data(), s.length());
    }
    bool endsWith(char) const;
    bool endsWith(char32_t cp) const;
/* hash.cpp */
    uint32_t hashCode() const;
private:
/* indexOf.cpp */
    int32_t indexOfInternal(const char*, uint32_t) const;
public:
    template<typename T> enable_if_ptr<T, char, int32_t> indexOf(T&& str) const {
        return indexOfInternal(str, strlen(str));
    }
    template<int32_t LITLEN> int32_t indexOf(const char (&literal)[LITLEN]) const {
        return indexOfInternal(literal, LITLEN-1);
    }
    int32_t indexOf(const string& s) const {
        return indexOfInternal(s.data(), s.length());
    }
    int32_t indexOf(char) const;
    int32_t indexOf(char32_t) const;

    template<typename T> enable_if_ptr<T, char, bool> includes(T&& str) const {
        return indexOf(str) != -1;
    }
    template<int32_t LITLEN> bool includes(const char (&literal)[LITLEN]) const {
        return indexOf(literal) != -1;
    }
    bool includes(const string& s) const {
        return indexOf(s) != -1;
    }
    bool includes(char ch) const {
        return indexOf(ch) != -1;
    }
    bool includes(char32_t cp) const {
        return indexOf(cp) != -1;
    }
private:
    int32_t lastIndexOfInternal(const char*, uint32_t) const;
public:
    template<typename T> enable_if_ptr<T, char, int32_t> lastIndexOf(T&& str) const {
        return lastIndexOfInternal(str, strlen(str));
    }
    template<int32_t LITLEN> int32_t lastIndexOf(const char (&literal)[LITLEN]) const {
        return lastIndexOfInternal(literal, LITLEN-1);
    }
    int32_t lastIndexOf(const string& s) const {
        return lastIndexOfInternal(s.data(), s.length());
    }
    int32_t lastIndexOf(char) const;
    int32_t lastIndexOf(char32_t) const;
private:
    uint32_t countOfInternal(const char*, uint32_t) const;
public:
    template<typename T> enable_if_ptr<T, char, uint32_t> countOf(T&& str) const {
        return countOfInternal(str, strlen(str));
    }
    template<int32_t LITLEN> uint32_t countOf(const char (&literal)[LITLEN]) const {
        return countOfInternal(literal, LITLEN-1);
    }
    uint32_t countOf(const string& s) const {
        return countOfInternal(s.data(), s.length());
    }
    uint32_t countOf(char) const;
    uint32_t countOf(char32_t) const;
private:
    static int32_t stringlib_count(
        const char* hay, int32_t hlen, 
        const char* needle, int32_t nlen, 
        int32_t maxcount
    );
    static int32_t stringlib_find(
        const char* hay, int32_t hlen,
        const char* needle, int32_t nlen,
        int32_t maxcount
    );
private:
/* transmogrify.cpp */
    string stringlib_expandtabs_impl(int tabsize) const;
    string pad(int32_t left, int32_t right, char fill) const;
    string stringlib_replace_interleave(const char* to_s, int32_t to_len, int32_t maxcount) const;
    string stringlib_replace_delete_single_character(char from_c, int32_t maxcount) const;
    string stringlib_replace_delete_substring(const char *from_s, int32_t from_len, int32_t maxcount) const;
    string stringlib_replace_single_character_in_place(char from_c, char to_c, int32_t maxcount) const;
    string stringlib_replace_substring_in_place(
        const char *from_s, int32_t from_len,
        const char *to_s, int32_t to_len,
        int32_t maxcount
    ) const;
    string stringlib_replace_single_character(
        char from_c, const char *to_s, 
        int32_t to_len, int32_t maxcount
    ) const;
    string stringlib_replace_substring(
        const char *from_s, int32_t from_len,
        const char *to_s, int32_t to_len,
        int32_t maxcount
    ) const;
    string stringlib_replace(
        const char *from_s, int32_t from_len,
        const char *to_s, int32_t to_len,
        int32_t maxcount
    ) const;
public:
    // five types: T&& (const char*), const char(&)[LITLEN], const string&, char, char32_t.
    template<typename T> enable_if_ptr<T, char, string> 
    replace(T&& from, T&& to) const {
        return stringlib_replace(
            from, strlen(from), 
            to, strlen(to), INT32_MAX
        );
    }
    template<typename T, int32_t LITLEN> enable_if_ptr<T, char, string> 
    replace(T&& from, const char (&to_literal)[LITLEN]) const {
        return stringlib_replace(
            from, strlen(from),
            to_literal, LITLEN, INT32_MAX
        );
    }
    template<typename T> enable_if_ptr<T, char, string> 
    replace(T&& from, const string& to) const {
        return stringlib_replace(
            from, strlen(from),
            to.data(), to.length(), INT32_MAX
        );
    }
    template<typename T> enable_if_ptr<T, char, string> 
    replace(T&& from, char to) const {
        return stringlib_replace(
            from, strlen(from),
            &to, 1, INT32_MAX
        );
    }
    template<typename T> enable_if_ptr<T, char, string> 
    replace(T&& from, char32_t to) const {
        char buf[5];
        return stringlib_replace(
            from, strlen(from),
            buf, cp2utf8(buf, to)
        );
    }
	
    template<int32_t LITLEN, typename T> enable_if_ptr<T, char, string> 
    replace(const char(&from_literal)[LITLEN], T&& to) const {
        return stringlib_replace(
            from_literal, LITLEN,
            to, strlen(to), INT32_MAX
        );
    }
    template<int32_t LITLEN1, int32_t LITLEN2> string
    replace(const char(&from_literal)[LITLEN1], const char(&to_literal)[LITLEN2]) const {
        return stringlib_replace(
            from_literal, LITLEN1,
            to_literal, LITLEN2, INT32_MAX
        );
    }
    template<int32_t LITLEN> string
    replace(const char(&from_literal)[LITLEN], const string& to) const {
        return stringlib_replace(
            from_literal, LITLEN,
            to.data(), to.length(), INT32_MAX
        );
    }
    template<int32_t LITLEN> string
    replace(const char(&from_literal)[LITLEN], char to) const {
        return stringlib_replace(
            from_literal, LITLEN,
            &to, 1, INT32_MAX
        );
    }
    template<int32_t LITLEN> string
    replace(const char(&from_literal)[LITLEN], char32_t to) const {
        char buf[5];
        return stringlib_replace(
            from_literal, LITLEN,
            buf, cp2utf8(buf, to), INT32_MAX
        );
    }
    
    template<typename T> enable_if_ptr<T, char, string> 
    replace(const string& from, T&& to) const {
        return stringlib_replace(
            from.data(), from.length(),
            to, strlen(to), INT32_MAX
        );
    }
    template<int32_t LITLEN> string
    replace(const string& from, const char (&to_literal)[LITLEN]) const {
        return stringlib_replace(
            from.data(), from.length(),
            to_literal, LITLEN, INT32_MAX
        );
    }
    string
    replace(const string& from, const string& to) const {
        return stringlib_replace(
            from.data(), from.length(),
            to.data(), to.length(), INT32_MAX
        );
    }
    string
    replace(const string& from, char to) const {
        return stringlib_replace(
            from.data(), from.length(),
            &to, 1, INT32_MAX
        );
    }
    string
    replace(const string& from, char32_t to) const {
        char buf[5];
        return stringlib_replace(
            from.data(), from.length(),
            buf, cp2utf8(buf, to), INT32_MAX
        );
    }

    template<typename T> enable_if_ptr<T, char, string> 
    replace(char from, T&& to) const {
        return stringlib_replace(
            &from, 1, 
            to, strlen(to), INT32_MAX
        );
    }
    template<int32_t LITLEN> string
    replace(char from, const char (&to_literal)[LITLEN]) const {
        return stringlib_replace(
            &from, 1,
            to_literal, LITLEN, INT32_MAX
        );
    }
    string
    replace(char from, const string& to) const {
        return stringlib_replace(
            &from, 1, 
            to.data(), to.length(), INT32_MAX
        );
    }
    string
    replace(char from, char to) const {
        return stringlib_replace_single_character_in_place(
            from, to, INT32_MAX
        );
    }
    string
    replace(char from, char32_t to) const {
        char buf[5];
        return stringlib_replace(
            &from, 1, 
            buf, cp2utf8(buf, to), INT32_MAX
        );
    }

    template<typename T> enable_if_ptr<T, char, string> 
    replace(char32_t from, T&& to) const {
        char buf[5];
        return stringlib_replace(
            buf, cp2utf8(buf, from),
            to, strlen(to), INT32_MAX
        );
    }
    template<int32_t LITLEN> string
    replace(char32_t from, const char (&to_literal)[LITLEN]) const {
        char buf[5];
        return stringlib_replace(
            buf, cp2utf8(buf, from),
            to_literal, LITLEN, INT32_MAX
        );
    }
    string
    replace(char32_t from, const string& to) const {
        char buf[5];
        return stringlib_replace(
            buf, cp2utf8(buf, from),
            to.data(), to.length(), INT32_MAX
        );
    }
    string
    replace(char32_t from, char to) const {
        char buf[5];
        return stringlib_replace(
            buf, cp2utf8(buf, from),
            &to, 1, INT32_MAX
        );
    }
    string
    replace(char32_t from, char32_t to) const {
        char buf1[5];
        char buf2[5];
        return stringlib_replace(
            buf1, cp2utf8(buf1, from),
            buf2, cp2utf8(buf2, to), INT32_MAX
        );
    }
/* misc.cpp */
    string padLeft(uint32_t max_len, char) const;
	string padRight(uint32_t max_len, char) const;
    int64_t parseInt();
	float parseFloat();
	double parseDouble();
    char* toCharArray() const;
    std::string toStl() const;
    std::vector<char> toVec() const;
    friend std::ostream& operator<<(std::ostream&, const string&);
private:
/* toCase.cpp */
    string caseMapUtf8(int mode) const;
public:
    string toUpperCase() const;
	string toTitleCase() const;
	string toLowerCase() const;
    string capitalize() const;
/* trim.cpp */
    string trim() const;
    string trimLeft() const;
    string trimRight() const;
/* valid.cpp */
    bool isUtf8() const;
    string toUtf8() const;
};

#endif