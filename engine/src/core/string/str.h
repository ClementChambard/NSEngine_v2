/** @file str.h
 * @brief Part of NSEngine String library.
 * @author Clement Chambard
 * @date 2024
 */

#ifndef STR_HEADER_INCLUDED
#define STR_HEADER_INCLUDED

#include "../slice.h"

namespace ns {

/** @class Str
 * @brief A separate class for string slices.
 *
 * This adds functonnalities to the Slice class that are specific to strings.
 */
class Str : public Slice<char> {
public:
  /**
   * @brief Creates a string slice from a C string.
   * @param s the C string to convert.
   */
  NS_API explicit Str(cstr);

  /**
   * @brief Conversion constructor from a character slice.
   * @param s the slice to convert.
   */
  NS_API Str(Slice<char> s) : Slice<char>(s) {}
};

} // namespace ns

#endif // STR_HEADER_INCLUDED
