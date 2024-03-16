#include "./test_manager.h"

#include <containers/vec.h>
#include <core/clock.h>
#include <core/logger.h>
#include <core/string.h>

struct test_entry {
  PFN_test func;
  cstr desc;
};

static ns::Vec<test_entry> tests;

void test_manager_register_test(u8 (*fn)(), cstr desc) {
  tests.push({fn, desc});
}

void test_manager_run_tests() {
  u32 passed = 0;
  u32 failed = 0;
  u32 skipped = 0;

  u32 count = static_cast<u32>(tests.len());

  ns::clock_t total_time;
  total_time.start();

  for (u32 i = 0; i < count; i++) {
    ns::clock_t test_time;
    test_time.start();
    u8 result = tests[i].func();
    test_time.update();

    if (result == true) {
      passed++;
    } else if (result == BYPASS) {
      NS_WARN("[SKIPPED]: %s", tests[i].desc);
      skipped++;
    } else {
      NS_ERROR("[FAILED]: %s", tests[i].desc);
      failed++;
    }
    char status[20];
    ns::string_fmt(status, sizeof(status),
                   failed ? "*** %d FAILED ***" : "SUCCESS", failed);
    total_time.update();
    NS_INFO("Executed %d of %d (skipped %d) %s (%.6f sec / %.6f sec total)",
            i + 1, count, skipped, status, test_time.elapsed,
            total_time.elapsed);
  }

  total_time.stop();

  NS_INFO("Results: %d passed, %d failed, %d skipped.", passed, failed,
          skipped);
}
