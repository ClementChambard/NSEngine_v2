#include <core/logger.h>
#include <math/ns_math.h>

#define expect(expected, actual)                                               \
  if (actual != expected) {                                                    \
    NS_ERROR("--> Expected %lld, but got: %lld. File: %s:%d.", expected,       \
             actual, __FILE__, __LINE__);                                      \
    return false;                                                              \
  }

#define expect_not(expected, actual)                                           \
  if (actual == expected) {                                                    \
    NS_ERROR("--> Expected %d != %d, but they are equal. File: %s:%d.",        \
             expected, actual, __FILE__, __LINE__);                            \
    return false;                                                              \
  }

#define expect_f(expected, actual)                                             \
  if (ns::abs(actual - expected) > 0.001f) {                                   \
    NS_ERROR("--> Expected %f, but got: %f. File: %s:%d.", expected, actual,   \
             __FILE__, __LINE__);                                              \
    return false;                                                              \
  }

#define expect_true(actual)                                                    \
  if (!(actual)) {                                                             \
    NS_ERROR("--> Expected true, but got: false. File: %s:%d.", __FILE__,      \
             __LINE__);                                                        \
    return false;                                                              \
  }

#define expect_false(actual)                                                   \
  if (actual) {                                                                \
    NS_ERROR("--> Expected false, but got: true. File: %s:%d.", __FILE__,      \
             __LINE__);                                                        \
    return false;                                                              \
  }
