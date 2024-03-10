#ifndef NS_STRING_HEADER_INCLUDED
#define NS_STRING_HEADER_INCLUDED

#include "../defines.h"

namespace ns {

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
NS_API str string_dup(cstr s);

/**
 * Clears a string
 * @param s the string to clear
 * @returns the cleared string
 */
NS_API str string_clear(str s);

/**
 * Formats a string
 * @param out the output string
 * @param n the maximum number of characters to write
 * @param format the format string
 * @returns the number of characters written
 */
NS_API i32 string_fmt(str out, usize n, cstr format, ...);

/**
 * Formats a string (va_list version)
 * @param out the output string
 * @param n the maximum number of characters to write
 * @param format the format string
 * @param va_list the variable argument list
 * @returns the number of characters written
 */
NS_API i32 string_fmt_v(str out, usize n, cstr format,
                        __builtin_va_list va_list);

/**
 * Copies a string
 * @param dest the destination string
 * @param src the source string
 * @returns the destination string
 */
NS_API str string_cpy(str dest, cstr src);

/**
 * Copies a string (n characters)
 * @param dest the destination string
 * @param src the source string
 * @param n the number of characters to copy
 * @returns the destination string
 */
NS_API str string_ncpy(str dest, cstr src, usize n);

/**
 * Trims a string
 * @param s the string to trim
 * @returns the trimmed string
 */
NS_API str string_trim(str s);

/**
 * Substring
 * @param dest the destination string
 * @param src the source string
 * @param start the start index
 * @param length the length
 */
NS_API void string_sub(str dest, cstr src, usize start, isize length);

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
