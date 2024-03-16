#include "./clock.h"

#include "../platform/platform.h"

namespace ns {

void clock_t::update() {
  if (start_time != 0) {
    elapsed = platform::get_absolute_time() - start_time;
  }
}

void clock_t::start() {
  start_time = platform::get_absolute_time();
  elapsed = 0.0;
}

void clock_t::stop() { start_time = 0.0; }

} // namespace ns
