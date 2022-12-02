# string
According to some people on the internet, every newbie to c++ should write a string class. This is my attempt.
- Immutable, atomically reference-counted for simplicity and convenience. No need for `const string&`.
- Easily convertable to a `cstr()`.
- `+` and `+=` operators implemented for `string`, `const char*`, all `intN_t` & `uintN_t`, `float`, `double`.
- `+=` in a loop has `std::vector` performance characteristics due to appending in-place with singly-referenced strings.
- Familiar functions `compareTo`, `startsWith`, `endsWith`, `includes`, `indexOf`, `indexOfNot`, `lastIndexOf`, `lastIndexOfNot`, `padLeft`, `padRight`, `parseInt`, `parseFloat`, `parseDouble`, `repeat`, `replace`, `reverse`, `split`, `substring`, `toCharArray`, `toUpperCase`, `toTitleCase`, `toLowerCase`, `trim`, `trimLeft`, `trimRight`.
- `substring` is `O(1)` due to reference counting.
- Haskell basic list operations `drop`, `head`, `init`, `last`, `tail`, `take`.
- Functional operations `filter`, `forEach`, `map`, `reduce`, `reduceRight`.
- No support for UTF-8.
- Very unoptimized, especially searching operations such as `indexOf`.
