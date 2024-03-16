#ifndef NS_STRING_HEADER_INCLUDED
#define NS_STRING_HEADER_INCLUDED

#include "../containers/vec.h"
#include "../defines.h"
#include "./memory.h"
#include "./slice.h"

namespace ns {

struct Str : public Slice<char> {
  explicit Str(cstr);
  Str(Slice<char> s) : Slice<char>(s) {}
};

struct String : public Vec<char, MemTag::STRING> {
  String(cstr s);
  void resize(usize n, char c = '\0');
  void push(char c);
  void push(cstr s);
  void push(Str s);
  void push(String const &s);
  char pop();
  void erase(char const *it) { erase(static_cast<usize>(it - m_data)); }
  void erase(usize index);
  cstr c_str() const { return m_data; }
};

/**
 * Checks if two strings are equal
 * @param s1 the first string
 * @param s2 the second string
 * @returns true if the strings are equal
 */
NS_API bool string_eq(cstr s1, cstr s2);

/**
 * Checks if two strings are equal (case insensitive)
 * @param s1 the first string
 * @param s2 the second string
 * @returns true if the strings are equal
 */
NS_API bool string_EQ(cstr s1, cstr s2);

/**
 * Returns the length of a string
 * @param s the string
 * @returns the length of the string
 */
NS_API usize string_length(cstr s);

/**
 * Copies a string
 * @param s the string to copy
 * @returns a copy of the string
 */
NS_API pstr string_dup(cstr s);

/**
 * Clears a string
 * @param s the string to clear
 * @returns the cleared string
 */
NS_API pstr string_clear(pstr s);

/**
 * Formats a string
 * @param out the output string
 * @param n the maximum number of characters to write
 * @param format the format string
 * @returns the number of characters written
 */
NS_API i32 string_fmt(pstr out, usize n, cstr format, ...);

/**
 * Formats a string (va_list version)
 * @param out the output string
 * @param n the maximum number of characters to write
 * @param format the format string
 * @param va_list the variable argument list
 * @returns the number of characters written
 */
NS_API i32 string_fmt_v(pstr out, usize n, cstr format,
                        __builtin_va_list va_list);

/**
 * Copies a string
 * @param dest the destination string
 * @param src the source string
 * @returns the destination string
 */
NS_API pstr string_cpy(pstr dest, cstr src);

/**
 * Copies a string (n characters)
 * @param dest the destination string
 * @param src the source string
 * @param n the number of characters to copy
 * @returns the destination string
 */
NS_API pstr string_ncpy(pstr dest, cstr src, usize n);

/**
 * Trims a string
 * @param s the string to trim
 * @returns the trimmed string
 */
NS_API pstr string_trim(pstr s);

/**
 * Substring
 * @param dest the destination string
 * @param src the source string
 * @param start the start index
 * @param length the length
 */
NS_API void string_sub(pstr dest, cstr src, usize start, isize length);

/**
 * Index of
 * @param s the string
 * @param c the character
 * @returns the index of the character in the string or -1 if not found
 */
NS_API isize string_indexof(cstr s, char c);

// temp
NS_API i32 string_scanf(cstr s, cstr format, ...);

NS_API bool string_parse(cstr s, f32 *out);
NS_API bool string_parse(cstr s, f64 *out);
NS_API bool string_parse(cstr s, i8 *out);
NS_API bool string_parse(cstr s, i16 *out);
NS_API bool string_parse(cstr s, i32 *out);
NS_API bool string_parse(cstr s, i64 *out);
NS_API bool string_parse(cstr s, u8 *out);
NS_API bool string_parse(cstr s, u16 *out);
NS_API bool string_parse(cstr s, u32 *out);
NS_API bool string_parse(cstr s, u64 *out);
NS_API bool string_parse(cstr s, bool *out);

} // namespace ns

#endif // NS_STRING_HEADER_INCLUDED
