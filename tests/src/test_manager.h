#ifndef TEST_MANAGER_HEADER_INCLUDED
#define TEST_MANAGER_HEADER_INCLUDED

#include <defines.h>

#define BYPASS 2

typedef u8 (*PFN_test)();

#define LAZY_REGISTER(fn) test_manager_register_test(fn, #fn)

void test_manager_register_test(PFN_test, cstr desc);

void test_manager_run_tests();

#endif // TEST_MANAGER_HEADER_INCLUDED
