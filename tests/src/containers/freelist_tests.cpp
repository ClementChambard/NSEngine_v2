#include "./freelist_tests.h"

#include "../expect.h"
#include "../test_manager.h"

#include <containers/freelist.h>
#include <core/logger.h>
#include <core/memory.h>

using ns::freelist;

u8 freelist_should_create_and_destroy() {
  NS_DEBUG("The following warning message is intentional.");
  freelist list;
  u64 mem_req = 0;
  u64 tot_siz = 40;
  ns::freelist_create(tot_siz, &mem_req, nullptr, nullptr);
  void *block = ns::alloc(mem_req, ns::MemTag::APPLICATION);
  ns::freelist_create(tot_siz, &mem_req, block, &list);

  expect_not(nullptr, list.memory);
  u64 free_space = freelist_free_space(list);
  expect(tot_siz, free_space);
  freelist_destroy(&list);
  expect(nullptr, list.memory);
  ns::free(block, mem_req, ns::MemTag::APPLICATION);

  return true;
}

u8 freelist_should_allocate_one_and_free_one() {
  freelist list;
  u64 mem_req = 0;
  u64 tot_siz = 512;
  ns::freelist_create(tot_siz, &mem_req, nullptr, nullptr);
  void *block = ns::alloc(mem_req, ns::MemTag::APPLICATION);
  ns::freelist_create(tot_siz, &mem_req, block, &list);

  u64 offset = INVALID_ID;
  bool result = ns::freelist_allocate_block(list, 64, &offset);
  expect_true(result);
  expect(0, offset);

  u64 freespace = freelist_free_space(list);
  expect(tot_siz - 64, freespace);

  result = ns::freelist_free_block(list, 64, offset);
  expect_true(result);

  freespace = freelist_free_space(list);
  expect(tot_siz, freespace);

  ns::freelist_destroy(&list);
  expect(nullptr, list.memory);
  ns::free(block, mem_req, ns::MemTag::APPLICATION);

  return true;
}

u8 freelist_should_allocate_one_and_free_multi() {
  freelist list;
  u64 mem_req = 0;
  u64 tot_siz = 512;
  ns::freelist_create(tot_siz, &mem_req, nullptr, nullptr);
  void *block = ns::alloc(mem_req, ns::MemTag::APPLICATION);
  ns::freelist_create(tot_siz, &mem_req, block, &list);
  u64 offset = INVALID_ID;
  bool result = ns::freelist_allocate_block(list, 64, &offset);
  expect_true(result);
  expect(0, offset);
  u64 offset2 = INVALID_ID;
  result = ns::freelist_allocate_block(list, 64, &offset2);
  expect_true(result);
  expect(64, offset2);
  u64 offset3 = INVALID_ID;
  result = ns::freelist_allocate_block(list, 64, &offset3);
  expect_true(result);
  expect(128, offset3);
  u64 free_space = freelist_free_space(list);
  expect(tot_siz - 192, free_space);
  result = ns::freelist_free_block(list, 64, offset2);
  expect_true(result);
  free_space = freelist_free_space(list);
  expect(tot_siz - 128, free_space);
  u64 offset4 = INVALID_ID;
  result = ns::freelist_allocate_block(list, 64, &offset4);
  expect_true(result);
  expect(offset2, offset4);
  free_space = freelist_free_space(list);
  expect(tot_siz - 192, free_space);
  result = ns::freelist_free_block(list, 64, offset);
  expect_true(result);
  free_space = freelist_free_space(list);
  expect(tot_siz - 128, free_space);
  result = ns::freelist_free_block(list, 64, offset3);
  expect_true(result);
  free_space = freelist_free_space(list);
  expect(tot_siz - 64, free_space);
  result = ns::freelist_free_block(list, 64, offset4);
  expect_true(result);
  free_space = freelist_free_space(list);
  expect(tot_siz, free_space);
  ns::freelist_destroy(&list);
  expect(nullptr, list.memory);
  ns::free(block, mem_req, ns::MemTag::APPLICATION);
  return true;
}

u8 freelist_should_allocate_one_and_free_multi_varying_sizes() {
  // TODO
  return UNIMPLEMENTED;
}

u8 freelist_should_allocate_to_full_and_fail_to_allocate_more() {

  freelist list;
  u64 mem_req = 0;
  u64 tot_siz = 512;
  ns::freelist_create(tot_siz, &mem_req, nullptr, nullptr);
  void *block = ns::alloc(mem_req, ns::MemTag::APPLICATION);
  ns::freelist_create(tot_siz, &mem_req, block, &list);
  u64 offset = INVALID_ID;
  bool result = ns::freelist_allocate_block(list, 512, &offset);
  expect_true(result);
  expect(0, offset);
  u64 offset2 = INVALID_ID;
  NS_DEBUG("The following warning message is intentional.");
  result = ns::freelist_allocate_block(list, 64, &offset2);
  expect_false(result);
  ns::freelist_destroy(&list);
  expect(nullptr, list.memory);
  ns::free(block, mem_req, ns::MemTag::APPLICATION);
  return true;
}

void freelist_register_tests() {
  test_manager_register_test(freelist_should_create_and_destroy,
                             "Freelist should create and destroy");
  test_manager_register_test(freelist_should_allocate_one_and_free_one,
                             "Freelist should allocate one and free one");
  test_manager_register_test(freelist_should_allocate_one_and_free_multi,
                             "Freelist should allocate one and free multi");
  test_manager_register_test(
      freelist_should_allocate_one_and_free_multi_varying_sizes,
      "Freelist should allocate one and free multi varying sizes");
  test_manager_register_test(
      freelist_should_allocate_to_full_and_fail_to_allocate_more,
      "Freelist should allocate to full and fail to allocate more");
}
