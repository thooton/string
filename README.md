# string
This project attempts to provide a string type that is both convenient and performant.
- Immutable and atomically reference-counted (no need for `const string&`).
- Short-string optimization performed on strings less than `sizeof(string)` - 16 bytes on 64-bit systems and 12 bytes on 32-bit systems.
- Construction with string literals (`string val = "..."`) is `O(1)`, thanks to C++ template string literals (`template <uint32_t LITLEN> const char(&)[LITLEN]`).
- `substring` is `O(1)` due to reference counting.
- Searching algorithms used in `countOf`, `indexOf`, `lastIndexOf`, `includes`, `replace` are optimized as they are taken from CPython.
- `toUpperCase`, `toTitleCase`, `toLowerCase`, `capitalize`, `trim*` methods have full UTF-8 support, although it is not required; invalid UTF-8 is safe with all methods. 
- `+` and `+=` operators implemented for many data types, including integral and floating-point, as well as `char` and `char32_t`.
- `+=` in a loop has `std::vector` performance characteristics due to appending in-place with singly-referenced strings.

To compile, simply compile all *.cpp files in the `src` directory (but not any of its subdirectories).