#include "./test_manager.h"

#include "./containers/freelist_tests.h"
#include "./containers/hashtable_tests.h"
#include "./memory/linear_allocator_tests.h"

#include <core/logger.h>

int main() {
  NS_DEBUG("Starting tests...");

  linear_allocator_register_tests();
  hashtable_register_tests();
  freelist_register_tests();
  NS_WARN("Dynamic allocator tests not implemented. TODO!");

  test_manager_run_tests();

  return 0;
}
