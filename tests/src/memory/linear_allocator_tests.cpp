#include "./linear_allocator_tests.h"
#include "../expect.h"
#include "../test_manager.h"

#include <defines.h>
#include <memory/linear_allocator.h>

using ns::linear_allocator;

u8 linear_allocator_should_create_and_destroy() {
  linear_allocator alloc(sizeof(u64), 0);

  expect_not(nullptr, alloc.memory);
  expect(sizeof(u64), alloc.total_size);
  expect(0, alloc.allocated);

  alloc.~linear_allocator();

  expect(0, alloc.memory);
  expect(0, alloc.total_size);
  expect(0, alloc.allocated);

  return true;
}

u8 linear_allocator_single_allocation_all_space() {
  linear_allocator alloc(sizeof(u64), 0);
  ptr block = alloc.allocate(sizeof(u64));
  expect_not(nullptr, block);
  expect(sizeof(u64), alloc.allocated);
  return true;
}

u8 linear_allocator_multi_allocation_all_space() {
  u64 max_allocs = 1024;
  linear_allocator alloc(sizeof(u64) * max_allocs, 0);

  ptr block;
  for (u64 i = 0; i < max_allocs; i++) {
    block = alloc.allocate(sizeof(u64));

    expect_not(nullptr, block);
    expect(sizeof(u64) * (i + 1), alloc.allocated);
  }

  return true;
}

u8 linear_allocator_multi_allocation_over_allocate() {
  u64 max_allocs = 3;
  linear_allocator alloc(sizeof(u64) * max_allocs, 0);

  ptr block;
  for (u64 i = 0; i < max_allocs; i++) {
    block = alloc.allocate(sizeof(u64));
    expect_not(nullptr, block);
    expect(sizeof(u64) * (i + 1), alloc.allocated);
  }

  NS_DEBUG("Note: The following error is intentionnally caused by this test.");

  block = alloc.allocate(sizeof(u64));
  expect(nullptr, block);
  expect(sizeof(u64) * max_allocs, alloc.allocated);

  return true;
}

u8 linear_allocator_multi_allocation_all_space_then_free() {
  u64 max_allocs = 1024;
  linear_allocator alloc(sizeof(u64) * max_allocs, 0);

  ptr block;
  for (u64 i = 0; i < max_allocs; i++) {
    block = alloc.allocate(sizeof(u64));
    expect_not(nullptr, block);
    expect(sizeof(u64) * (i + 1), alloc.allocated);
  }

  alloc.free_all();
  expect(0, alloc.allocated);

  return true;
}

void linear_allocator_register_tests() {
  test_manager_register_test(linear_allocator_should_create_and_destroy,
                             "Linear allocator should create and destroy");
  test_manager_register_test(linear_allocator_single_allocation_all_space,
                             "Linear allocator single allocation all space");
  test_manager_register_test(linear_allocator_multi_allocation_all_space,
                             "Linear allocator multi allocation all space");
  test_manager_register_test(linear_allocator_multi_allocation_over_allocate,
                             "Linear allocator multi allocation over allocate");
  test_manager_register_test(
      linear_allocator_multi_allocation_all_space_then_free,
      "Linear allocator multi allocation all space then free");
}
