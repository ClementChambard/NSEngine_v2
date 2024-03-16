#include "./hashtable_tests.h"
#include "../expect.h"
#include "../test_manager.h"

#include <containers/hashtable.h>
#include <defines.h>

using ns::chashtable;

u8 hashtable_should_create_and_destroy() {
  const usize element_size = sizeof(u64);
  const u64 element_count = 3;
  u64 memory[element_count];

  chashtable table(element_size, element_count, memory);

  expect_not(nullptr, table.memory);
  expect(sizeof(u64), table.element_size);
  expect(3, table.element_count);

  table.~chashtable();

  expect(nullptr, table.memory);
  expect(0, table.element_size);
  expect(0, table.element_count);

  return true;
}

u8 hashtable_should_set_and_get_successfully() {
  const usize element_size = sizeof(u64);
  const u64 element_count = 3;
  u64 memory[element_count];

  chashtable table(element_size, element_count, memory);

  expect_not(nullptr, table.memory);
  expect(sizeof(u64), table.element_size);
  expect(3, table.element_count);

  u64 testval1 = 23;
  table.set("test1", &testval1);
  u64 get_testval1 = 0;
  table.get("test1", &get_testval1);
  expect(23, get_testval1);

  table.~chashtable();

  expect(nullptr, table.memory);
  expect(0, table.element_size);
  expect(0, table.element_count);

  return true;
}

namespace {
struct test_struct {
  bool b_value;
  f32 f_value;
  u64 u_value;
};
} // namespace

u8 hashtable_should_set_and_get_ptr_successfully() {
  const usize element_size = sizeof(test_struct *);
  const u64 element_count = 3;
  test_struct *memory[element_count];

  chashtable table(element_size, element_count, memory, true);

  expect_not(nullptr, table.memory);
  expect(sizeof(test_struct *), table.element_size);
  expect(3, table.element_count);

  test_struct t;
  test_struct *testval1 = &t;
  t.b_value = true;
  t.f_value = 3.1415f;
  t.u_value = 63;
  table.set_ptr("test1", testval1);
  test_struct *get_testval1 = nullptr;
  table.get_ptr("test1", &get_testval1);

  expect(true, get_testval1->b_value);
  expect(3.1415f, get_testval1->f_value);
  expect(63, get_testval1->u_value);

  table.~chashtable();

  expect(nullptr, table.memory);
  expect(0, table.element_size);
  expect(0, table.element_count);

  return true;
}

u8 hashtable_should_set_and_get_nonexistant() {
  const usize element_size = sizeof(u64);
  const u64 element_count = 3;
  u64 memory[element_count];

  chashtable table(element_size, element_count, memory);

  expect_not(nullptr, table.memory);
  expect(sizeof(u64), table.element_size);
  expect(3, table.element_count);

  u64 testval1 = 23;
  table.set("test1", &testval1);
  u64 get_testval1 = 0;
  table.get("test2", &get_testval1);
  expect(0, get_testval1);

  table.~chashtable();

  expect(nullptr, table.memory);
  expect(0, table.element_size);
  expect(0, table.element_count);

  return true;
}

u8 hashtable_should_set_and_get_ptr_nonexistant() {
  const usize element_size = sizeof(test_struct *);
  const u64 element_count = 3;
  test_struct *memory[element_count];

  chashtable table(element_size, element_count, memory, true);

  expect_not(nullptr, table.memory);
  expect(sizeof(test_struct *), table.element_size);
  expect(3, table.element_count);

  test_struct t;
  test_struct *testval1 = &t;
  t.b_value = true;
  t.f_value = 3.1415f;
  t.u_value = 63;
  table.set_ptr("test1", testval1);
  test_struct *get_testval1 = nullptr;
  bool result = table.get_ptr("test2", &get_testval1);
  expect_false(result);
  expect(nullptr, get_testval1);

  table.~chashtable();

  expect(nullptr, table.memory);
  expect(0, table.element_size);
  expect(0, table.element_count);

  return true;
}

u8 hashtable_should_set_and_unset_ptr() {
  const usize element_size = sizeof(test_struct *);
  const u64 element_count = 3;
  test_struct *memory[element_count];

  chashtable table(element_size, element_count, memory, true);

  expect_not(nullptr, table.memory);
  expect(sizeof(test_struct *), table.element_size);
  expect(3, table.element_count);

  test_struct t;
  test_struct *testval1 = &t;
  t.b_value = true;
  t.f_value = 3.1415f;
  t.u_value = 63;
  bool result = table.set_ptr("test1", testval1);
  expect_true(result);
  test_struct *get_testval1 = nullptr;
  result = table.get_ptr("test1", &get_testval1);
  expect_true(result);
  expect(testval1->b_value, get_testval1->b_value);
  expect(testval1->f_value, get_testval1->f_value);
  expect(testval1->u_value, get_testval1->u_value);
  result = table.set_ptr("test1", nullptr);
  expect_true(result);

  get_testval1 = nullptr;
  result = table.get_ptr("test1", &get_testval1);
  expect_false(result);
  expect(nullptr, get_testval1);

  table.~chashtable();

  expect(nullptr, table.memory);
  expect(0, table.element_size);
  expect(0, table.element_count);

  return true;
}

u8 hashtable_try_call_non_ptr_on_ptr_table() {
  const usize element_size = sizeof(test_struct *);
  const u64 element_count = 3;
  test_struct *memory[element_count];

  chashtable table(element_size, element_count, memory, true);

  expect_not(nullptr, table.memory);
  expect(sizeof(test_struct *), table.element_size);
  expect(3, table.element_count);

  NS_DEBUG("The following 2 error messages are intentional.");

  test_struct t;
  t.b_value = true;
  t.f_value = 3.1415f;
  t.u_value = 63;
  bool result = table.set("test1", &t);
  expect_false(result);
  test_struct *get_testval1 = nullptr;
  result = table.get("test1", get_testval1);
  expect_false(result);

  table.~chashtable();

  expect(nullptr, table.memory);
  expect(0, table.element_size);
  expect(0, table.element_count);

  return true;
}

u8 hashtable_try_call_ptr_on_non_ptr_table() {
  const usize element_size = sizeof(u64);
  const u64 element_count = 3;
  u64 memory[element_count];

  chashtable table(element_size, element_count, memory);

  expect_not(nullptr, table.memory);
  expect(sizeof(test_struct *), table.element_size);
  expect(3, table.element_count);

  NS_DEBUG("The following 2 error messages are intentional.");

  test_struct t;
  t.b_value = true;
  t.f_value = 3.1415f;
  t.u_value = 63;
  bool result = table.set_ptr("test1", &t);
  expect_false(result);
  test_struct *get_testval1 = nullptr;
  result = table.get_ptr("test1", &get_testval1);
  expect_false(result);

  table.~chashtable();

  expect(nullptr, table.memory);
  expect(0, table.element_size);
  expect(0, table.element_count);

  return true;
}

u8 hashtable_should_set_get_and_update_ptr_successfully() {
  const usize element_size = sizeof(test_struct *);
  const u64 element_count = 3;
  test_struct *memory[element_count];

  chashtable table(element_size, element_count, memory, true);

  expect_not(nullptr, table.memory);
  expect(sizeof(test_struct *), table.element_size);
  expect(3, table.element_count);

  test_struct t;
  test_struct *testval1 = &t;
  t.b_value = true;
  t.f_value = 3.1415f;
  t.u_value = 63;
  table.set_ptr("test1", testval1);
  test_struct *get_testval1 = nullptr;
  table.get_ptr("test1", &get_testval1);
  expect(true, get_testval1->b_value);
  expect(3.1415f, get_testval1->f_value);
  expect(63, get_testval1->u_value);
  get_testval1->b_value = false;
  get_testval1->f_value = 2.71828f;
  get_testval1->u_value = 42;
  table.set_ptr("test1", testval1);
  test_struct *get_testval2 = nullptr;
  table.get_ptr("test1", &get_testval2);
  expect(false, get_testval2->b_value);
  expect(2.71828f, get_testval2->f_value);
  expect(42, get_testval2->u_value);

  table.~chashtable();

  expect(nullptr, table.memory);
  expect(0, table.element_size);
  expect(0, table.element_count);

  return true;
}

void hashtable_register_tests() {
  test_manager_register_test(hashtable_should_create_and_destroy,
                             "Hashtable should create and destroy");
  test_manager_register_test(hashtable_should_set_and_get_successfully,
                             "Hashtable should set and get successfully");
  test_manager_register_test(hashtable_should_set_and_get_ptr_successfully,
                             "Hashtable should set and get ptr successfully");
  test_manager_register_test(hashtable_should_set_and_get_nonexistant,
                             "Hashtable should set and get nonexistant");
  test_manager_register_test(hashtable_should_set_and_get_ptr_nonexistant,
                             "Hashtable should set and get ptr nonexistant");
  test_manager_register_test(hashtable_try_call_non_ptr_on_ptr_table,
                             "Hashtable try call non ptr on ptr table");
  test_manager_register_test(hashtable_try_call_ptr_on_non_ptr_table,
                             "Hashtable try call ptr on non ptr table");
  test_manager_register_test(
      hashtable_should_set_get_and_update_ptr_successfully,
      "Hashtable should set get and update ptr successfully");
}
