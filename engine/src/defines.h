#ifndef DEFINES_HEADER_INCLUDED
#define DEFINES_HEADER_INCLUDED

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned long long u64;

typedef signed char i8;
typedef signed short i16;
typedef signed int i32;
typedef signed long long i64;

typedef unsigned long long usize;
typedef signed long long isize;

typedef float f32;
typedef double f64;

typedef char const *cstr;
typedef char *str;
typedef void *ptr;
typedef const void *roptr;
typedef u8 byte;
typedef byte *bytes;
typedef const byte *robytes;
typedef u32 NSID;

#define AS_BYTES(x) reinterpret_cast<bytes>(x)

#if defined(__clang__) || defined(__gcc__)
#define NS_STATIC_ASSERT _Static_assert
#else
#define NS_STATIC_ASSERT static_assert
#endif

NS_STATIC_ASSERT(sizeof(u8) == 1, "Expected u8 to be 1 byte.");
NS_STATIC_ASSERT(sizeof(u16) == 2, "Expected u16 to be 2 byte.");
NS_STATIC_ASSERT(sizeof(u32) == 4, "Expected u32 to be 4 byte.");
NS_STATIC_ASSERT(sizeof(u64) == 8, "Expected u64 to be 8 byte.");

NS_STATIC_ASSERT(sizeof(i8) == 1, "Expected i8 to be 1 byte.");
NS_STATIC_ASSERT(sizeof(i16) == 2, "Expected i16 to be 2 byte.");
NS_STATIC_ASSERT(sizeof(i32) == 4, "Expected i32 to be 4 byte.");
NS_STATIC_ASSERT(sizeof(i64) == 8, "Expected i64 to be 8 byte.");

NS_STATIC_ASSERT(sizeof(usize) == 8, "Expected usize to be 8 byte.");
NS_STATIC_ASSERT(sizeof(isize) == 8, "Expected isize to be 8 byte.");

NS_STATIC_ASSERT(sizeof(f32) == 4, "Expected f32 to be 4 byte.");
NS_STATIC_ASSERT(sizeof(f64) == 8, "Expected f64 to be 8 byte.");

#define INVALID_ID 4294967295U

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__)
#define NS_PLATFORM_WINDOWS 1
#ifndef _WIN64
#error "64-bit is required on Windows!"
#endif
#elif defined(__linux__) || defined(__gnu_linux__)
#define NS_PLATFORM_LINUX 1
#if defined(__ANDROID__)
#define NS_PLATFORM_ANDROID 1
#endif
#elif defined(__unix__)
#define NS_PLATFORM_UNIX 1
#elif defined(_POSIX_VERSION)
#define NS_PLATFORM_POSIX 1
#elif __APPLE__
#define NS_PLATFORM_APPLE 1
#include <TargetConditionals.h>
#if TARGET_IPHONE_SIMULATOR
#define NS_PLATFORM_IOS 1
#define NS_PLATFORM_IOS_SIMULATOR 1
#elif TARGET_OS_IPHONE
#define NS_PLATFORM_IOS 1
#elif TARGET_OS_MAC
#else
#error "Unknown Apple platform"
#endif
#else
#error "Unknown platform!"
#endif

#ifdef NS_EXPORT
#ifdef _MSC_VER
#define NS_API __declspec(dllexport)
#else
#define NS_API __attribute__((visibility("default")))
#endif
#else
#ifdef _MSC_VER
#define NS_API __declspec(dllimport)
#else
#define NS_API
#endif
#endif

#ifdef _MSC_VER
#define NS_INLINE __forceinline
#define NS_NOINLINE __declspec(noinline)
#else
#define NS_INLINE static inline
#define NS_NOINLINE
#endif

#endif // DEFINES_HEADER_INCLUDED
