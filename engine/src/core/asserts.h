#ifndef ASSERTS_HEADER_INCLUDED
#define ASSERTS_HEADER_INCLUDED

#include "../defines.h"

// Disable assertions by commenting this line
#define NS_ASSERTIONS_ENABLED

#ifdef NS_ASSERTIONS_ENABLED
#if _MSC_VER
#include <intrin.h>
#define debugBreak() __debugbreak()
#else
#define debugBreak() __builtin_trap()
#endif

namespace ns {

void report_assertion_failure(cstr expression, cstr message, cstr file,
                              isize line);

}

#define NS_ASSERT(expr)                                                        \
  {                                                                            \
    if (expr) {                                                                \
    } else {                                                                   \
      ::ns::report_assertion_failure(#expr, "", __FILE__, __LINE__);           \
      debugBreak();                                                            \
    }                                                                          \
  }
#define NS_ASSERT_M(expr, message)                                             \
  {                                                                            \
    if (expr) {                                                                \
    } else {                                                                   \
      ::ns::report_assertion_failure(#expr, message, __FILE__, __LINE__);      \
      debugBreak();                                                            \
    }                                                                          \
  }

#ifdef _DEBUG
#define NS_ASSERT_DEBUG(expr)                                                  \
  {                                                                            \
    if (expr) {                                                                \
    } else {                                                                   \
      ::ns::report_assertion_failure(#expr, "", __FILE__, __LINE__);           \
      debugBreak();                                                            \
    }                                                                          \
  }
#define NS_ASSERT_DEBUG_M(expr, message)                                       \
  {                                                                            \
    if (expr) {                                                                \
    } else {                                                                   \
      ::ns::report_assertion_failure(#expr, message, __FILE__, __LINE__);      \
      debugBreak();                                                            \
    }                                                                          \
  }
#else
#define NS_ASSERT_DEBUG(expr)
#define NS_ASSERT_DEBUG_M(expr, message)
#endif

#else
#define NS_ASSERT(expr)
#define NS_ASSERT_M(expr, message)
#define NS_ASSERT_DEBUG(expr)
#define NS_ASSERT_DEBUG_M(expr, message)
#endif

#endif // ASSERTS_HEADER_INCLUDED
