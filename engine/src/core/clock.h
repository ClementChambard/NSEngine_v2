#ifndef CLOCK_HEADER_INCLUDED
#define CLOCK_HEADER_INCLUDED

#include "../defines.h"

namespace ns {

struct clock_t {
  f64 start_time;
  f64 elapsed;

  void update();
  void start();
  void stop();
};

} // namespace ns

#endif // CLOCK_HEADER_INCLUDED
