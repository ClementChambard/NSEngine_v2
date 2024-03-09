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

} // namespace ns

#endif // NS_STRING_HEADER_INCLUDED
