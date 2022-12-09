#include "string.hpp"
#include <atomic>
#include <stdlib.h>
#include <utility>

typedef uint64_t u64;
typedef int64_t i64;
typedef uint32_t u32;
typedef int32_t i32;
typedef uint16_t u16;
typedef int16_t i16;
typedef uint8_t u8;
typedef int8_t i8;

typedef std::atomic<u32> refc_t;

constexpr static u32 IS_LITTLE_ENDIAN_HELPER = 1;
constexpr static bool IS_LITTLE_ENDIAN = (const u8&)IS_LITTLE_ENDIAN_HELPER;

/* Allocated memory layout (least -> greatest ptr)
Reference count - sizeof(refc_t)
String data     - alloc length
Extra space     - alloc capacity - alloc length
Zero terminator - 1
Allocation size - sizeof(u32) 

Note: allocation size is little-endian */

#define SSO_CAP (sizeof(alloc)-1)
#define SSO_DATA ((char*)&alloc)
#define SSO_INFO (*(SSO_DATA + SSO_CAP))
#define SSO_DATA_FOR(obj) ((char*)&(obj)->alloc)

static void storeU32(char* dst, u32 val) {
    u8* dest = (u8*)dst;
    *(dest++) = (u8)val;
    val >>= 8;
    *(dest++) = (u8)val;
    val >>= 8;
    *(dest++) = (u8)val;
    val >>= 8;
    *(dest++) = (u8)val;
}
static u32 loadU32(char* src) {
    u8* source = (u8*)src;
    u32 val = *(source++);
    val |= (*(source++)) << 8;
    val |= (*(source++)) << 16;
    val |= (*(source++)) << 24;
    return val;
}
static char* allocWithFooter(u32 len) {
    u32 alloc_size = sizeof(refc_t) + len + 1 + 4;
    char* memory = (char*)malloc(alloc_size);
    new (memory) refc_t(1);
    storeU32(memory+alloc_size-4, alloc_size);
    return memory+sizeof(refc_t);
}
static u32 nextHighestPowerOfTwo(u32 v) {
    v--;
    v |= v >> 1;
    v |= v >> 2;
    v |= v >> 4;
    v |= v >> 8;
    v |= v >> 16;
    v++;
    return v;
}
typedef struct {
    char* data;
    u32 cap;
} Vec;
static Vec allocVectorWithFooter(u32 needed_cap) {
    u32 alloc_size = nextHighestPowerOfTwo(
        sizeof(refc_t) + needed_cap + 1 + 4
    );
    u32 actual_cap = alloc_size - sizeof(refc_t) - 1 - 4;
    char* memory = (char*)malloc(alloc_size);
    new (memory) refc_t(1);
    storeU32(memory+alloc_size-4, alloc_size);
    return Vec{memory+sizeof(refc_t), actual_cap};
}
static char* reallocNonSubstringWithFooter(char* data, u32 new_cap) {
    u32 alloc_size = sizeof(refc_t) + new_cap + 1 + 4;
    char* existing = data - sizeof(refc_t);
    char* memory = (char*)realloc(existing, alloc_size);
    storeU32(memory+alloc_size-4, alloc_size);
    return memory+sizeof(refc_t);
}
static refc_t* getRefCount(char* data, u32 cap) {
    char* size_ptr = data + cap + 1;
    u32 alloc_size = loadU32(size_ptr);
    return (refc_t*)(size_ptr + 4 - alloc_size);
}
#define GET_REF_COUNT() (getRefCount(alloc.data, getAllocCap()))

// If big endian, set (v<<1)|1 and retrieve v>>1.
// If little endian, set v with (top byte<<1)|1, retrieve v with top byte>>1.
void string::setAllocCap(u32 v) {
    if (IS_LITTLE_ENDIAN) {
        alloc.cap_info = (v & 0x00FFFFFF)
            | ((v & 0xFF000000) << 1) 
            | 0x01000000;
    } else {
        alloc.cap_info = (v << 1) | 1;
    }
}
u32 string::getAllocCap() const {
    u32 cap_info = alloc.cap_info;
    if (IS_LITTLE_ENDIAN) {
        return (cap_info & 0x00FFFFFF) 
            | ((cap_info & 0xFE000000) >> 1);
    } else {
        return cap_info >> 1;
    }
}
bool string::allocActive() const {
    return (SSO_INFO & 1)
        && getAllocCap() != 0;
}
bool string::ssoActive() const {
    return !(SSO_INFO & 1);
}
bool string::litActive() const {
    return (SSO_INFO & 1)
        && getAllocCap() == 0;
}
void string::setSsoLen(int v) {
    SSO_INFO = (SSO_CAP - v) << 1;
}
int string::getSsoLen() const {
    return SSO_CAP - (SSO_INFO >> 1);
}
char* string::data() const {
    if (ssoActive()) {
        return SSO_DATA;
    } else {
        return alloc.data;
    }
}
u32 string::length() const {
    if (ssoActive()) {
        return getSsoLen();
    } else {
        return alloc.len;
    }
}

string::string() {
    SSO_DATA[0] = '\0';
    setSsoLen(0);
}

string::string(const char* str, i32 len, bool is_literal) {
    if (len <= SSO_CAP) {
        memcpy(SSO_DATA, str, len);
        SSO_DATA[len] = 0;
        setSsoLen(len);
    } else if (is_literal) {
        alloc.data = (char*)str;
        alloc.len = len;
        setAllocCap(0);
    } else {
        char* data = allocWithFooter(len);
        memcpy(data, str, len);
        data[len] = '\0';
        alloc.data = data;
        alloc.len = len;
        setAllocCap(len);
    }
}
string::string(i32 uninit_len) {
    if (uninit_len <= SSO_CAP) {
        SSO_DATA[uninit_len] = '\0';
        setSsoLen(uninit_len);
    } else {
        char* data = allocWithFooter(uninit_len);
        data[uninit_len] = '\0';
        alloc.data = data;
        alloc.len = uninit_len;
        setAllocCap(uninit_len);
    }
}
string::string(
    const char* one, const char* two, 
    u32 one_len, u32 two_len
) {
    u32 total_len = one_len + two_len;
    if (total_len <= SSO_CAP) {
        memcpy(SSO_DATA, one, one_len);
        memcpy(SSO_DATA + one_len, two, two_len);
        SSO_DATA[total_len] = '\0';
        setSsoLen(total_len);
    } else {
        char* data = allocWithFooter(total_len);
        memcpy(data, one, one_len);
        memcpy(data + one_len, two, two_len);
        data[total_len] = '\0';
        alloc.data = data;
        alloc.len = total_len;
        setAllocCap(total_len);
    }
}
void string::incref() const {
    GET_REF_COUNT()->fetch_add(1, std::memory_order_relaxed);
}
void string::decref() const {
    refc_t* ref_count = GET_REF_COUNT();
    
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
    (void)ref_count->load(std::memory_order_acquire);

    /* now free() makes the writes visible on all threads
    * (I think) anyway,
    * gecko https://searchfox.org/mozilla-central/rev/ae8c2e2354db
    * 652950fe0ec16983360c21857f2a/xpcom/base/nsISupportsImpl.h#337
    * uses this pattern and it seems to work for them
    */
    free(ref_count);
}
u32 string::refcnt() const {
    return GET_REF_COUNT()->load(std::memory_order_relaxed);
}
string::string(const string* src, u32 start, u32 end) {
    u32 len = end - start;
    if (src->ssoActive()) {
        memcpy(
            &alloc, 
            SSO_DATA_FOR(src) + start,
            len
        );
        SSO_DATA[len] = '\0';
        setSsoLen(len);
    } else {
        alloc.data = src->alloc.data + start;
        alloc.len = len;
        alloc.cap_info = src->alloc.cap_info;
        if (allocActive()) {
            setAllocCap(getAllocCap() - start);
            incref();
        }
    }
}

string::~string() {
    if (allocActive()) {
        decref();
    }
}

string::string(const string& other) {
    memcpy(&alloc, &other.alloc, sizeof(alloc));
    if (allocActive()) {
        incref();
    }
}
string& string::operator=(const string& other) {
    if (allocActive()) {
        decref();
    }
    memcpy(&alloc, &other.alloc, sizeof(alloc));
    if (allocActive()) {
        incref();
    }
    return *this;
}

string::string(string&& other) {
    memcpy(&alloc, &other.alloc, sizeof(alloc));
    other.setSsoLen(0);
    SSO_DATA_FOR(&other)[0] = '\0';
}
string& string::operator=(string&& other) {
    if (allocActive()) {
        decref();
    }
    memcpy(&alloc, &other.alloc, sizeof(alloc));
    other.setSsoLen(0);
    SSO_DATA_FOR(&other)[0] = '\0';
    return *this;
}

const char* string::str() {
    if (ssoActive()) {
        return SSO_DATA;
    }
    if (alloc.data[alloc.len] == 0) {
        return alloc.data;
    }
    if (allocActive() && refcnt() == 1) {
        alloc.data[alloc.len] = 0;
        return alloc.data;
    }

    *this = string(alloc.data, alloc.len);
    return alloc.data;
}


void string::ensureSpaceFor(u32 more) {
    if (ssoActive()) {
        int sso_len = getSsoLen();
        u32 needed_cap = sso_len + more;
        if (needed_cap <= SSO_CAP) {
            return;
        }
        Vec res = allocVectorWithFooter(needed_cap);
        memcpy(res.data, SSO_DATA, sso_len+1);
        alloc.data = res.data;
        alloc.len = sso_len;
        setAllocCap(res.cap);
    } else {
        /* Here we might want to consider using realloc
         * instead of malloc, so that the allocator can
         * be more efficient, resizing the existing
         * memory block if necessary rather than
         * allocating a completely new one.
         *
         * There are considerations that might make this
         * less efficient. We may be a substring; however,
         * realloc, if it needs to, will copy the entire allocation,
         * not just the parts we need. If len < cap, or we are a substring
         * that starts later than the original allocation, time and memory
         * will be wasted (potentially a great deal of it).
         *
         * Another possibility is only using realloc() if we are a substring
         * starting at the original location.
         * 
         * It seems to me that the best option, or any sort of heuristic
         * to find the best option, can only really be determined by
         * benchmarking, and so I will leave it to the future.
        */
        u32 needed_cap = alloc.len + more;
        if (needed_cap <= getAllocCap()) {
            return;
        }
        Vec res = allocVectorWithFooter(needed_cap);
        memcpy(
            res.data, 
            alloc.data, 
            alloc.len
        );
        res.data[alloc.len] = '\0';
        alloc.data = res.data;
        setAllocCap(res.cap);
    }
}
void string::pushSingleton(const char* str, u32 len) {
    ensureSpaceFor(len);
    if (ssoActive()) {
        int sso_len = getSsoLen();
        memcpy(SSO_DATA+sso_len, str, len);
        int new_len = sso_len + len;
        SSO_DATA[new_len] = '\0';
        setSsoLen(new_len);
    } else {
        u32 alloc_len = alloc.len;
        memcpy(alloc.data+alloc_len, str, len);
        u32 new_len = alloc_len + len;
        alloc.data[new_len] = '\0';
        alloc.len = new_len;
    }
}
void string::pushSingletonChar(int val) {
    ensureSpaceFor(1);
    if (ssoActive()) {
        int sso_len = getSsoLen();
        SSO_DATA[sso_len] = val;
        SSO_DATA[sso_len+1] = '\0';
        setSsoLen(sso_len + 1);
    } else {
        u32 alloc_len = alloc.len;
        alloc.data[alloc_len] = val;
        alloc.data[alloc_len+1] = '\0';
        alloc.len = alloc_len;
    }
}
bool string::isSingleton() const {
    return ssoActive()
        || (allocActive() && refcnt() == 1);
}

string string::substring(u32 start) const {
    return string(this, start, length());
}
string string::substring(u32 start, u32 end) const {
    return string(this, start, end);
}

void string::shrinkNonSubstringToFitLength(u32 len) {
    if (len <= SSO_CAP) {
        if (!ssoActive()) {
            char* alloc_data = alloc.data;
            memcpy(
                SSO_DATA, 
                alloc_data,
                len
            );
            SSO_DATA[len] = '\0';
            free(alloc_data - sizeof(refc_t));
        }
        setSsoLen(len);
        return;
    }

    alloc.data = reallocNonSubstringWithFooter(
        alloc.data, len
    );
    alloc.data[len] = '\0';
    alloc.len = len;
    setAllocCap(len);
}