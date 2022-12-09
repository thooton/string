#include "string.hpp"
typedef uint32_t u32;

inline static u32 powU32(u32 a, u32 b) {
   u32 p = 1;
   while (b) {
	   if ((b&1) != 0) {
		   p *= a;
	   }
	   b >>= 1;
	   a *= a;
   }
   return p;
}

u32 string::hashCode() const {
	u32 n = length();
	u32 pwr = n-1;
	u32 hash = 0;
	for (size_t i = 0; i < n; i++, pwr--) {
		hash += (*this)[i] * powU32(31, pwr);
	}
	return hash;
}