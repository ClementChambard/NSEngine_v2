/** @file string.h
 * @brief Part of NSEngine String library.
 * @author Clement Chambard
 * @date 2024
 */

#ifndef NS_STRING_HEADER_INCLUDED
#define NS_STRING_HEADER_INCLUDED

#include "../containers/vec.h"
#include "../defines.h"
#include "./str.h"

namespace ns {

/** @class String
 * @brief String class created from Vec.
 *
 * This class is a wrapper around the Vec class that adds functonalities
 * specific to strings.
 */
class String : public Vec<char, MemTag::STRING> {
public:
  /**
   * @brief Creates a String from a C string.
   * @param s the C string to convert.
   */
  NS_API String(cstr s);

  /**
   * @brief Resizes the string.
   * @warning Does nothing since resizing a String might not be possible.
   */
  NS_API void resize(usize n, char c = '\0');

  /**
   * @brief Pushes a character to the string.
   * @param c the character to push.
   *
   * Method overloaded from the Vec class.
   */
  NS_API void push(char c);

  /**
   * @brief Pushes a C string to the string.
   * @param s the C string to push.
   */
  NS_API void push(cstr s);

  /**
   * @brief Pushes a string slice to the string.
   * @param s the string slice to push.
   */
  NS_API void push(Str s);

  /**
   * @brief Pushes a String to the string.
   * @param s the String to push.
   */
  NS_API void push(String const &s);

  /**
   * @brief Pops a character from the string.
   * @returns the popped character.
   *
   * Method overloaded from the Vec class.
   * The string must not be empty.
   */
  NS_API char pop();

  /**
   * @brief Erases a character from the string.
   * @param it an iterator to the character to erase
   *        (must be a valid iterator pointing to a character in the string).
   *
   * Method overloaded from the Vec class.
   */
  NS_API void erase(char const *it) { erase(static_cast<usize>(it - m_data)); }

  /**
   * @brief Erases a character from the string
   * @param index the index of the character to erase
   *        (must be a valid index from 0 to this->len() - 1).
   *
   * Method overloaded from the Vec class.
   */
  NS_API void erase(usize index);

  /** @fn cstr String::c_str() const
   * @brief Converts the string to a C string
   * @returns the C string corresponding to the string
   *          (returns nullptr if the string is empty).
   *          The returned pointer is valid until the string is modified or
   *          destroyed.
   */
  NS_API cstr c_str() const { return m_data; }

  // TODO: operators
};

} // namespace ns

#endif // NS_STRING_HEADER_INCLUDED
