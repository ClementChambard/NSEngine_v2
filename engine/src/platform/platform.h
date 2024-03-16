#ifndef PLATFORM_HEADER_INCLUDED
#define PLATFORM_HEADER_INCLUDED

#include "../defines.h"

namespace ns::platform {

bool startup(usize *memory_requirement, ptr state, cstr application_name, i32 x,
             i32 y, i32 width, i32 height);

void shutdown(ptr state);

bool pump_messages();

ptr allocate_memory(usize size, bool aligned);
ptr reallocate_memory(ptr block, usize new_size, bool aligned);
void free_memory(ptr block, bool aligned);
ptr zero_memory(ptr block, usize size);
ptr copy_memory(ptr dest, roptr source, usize size);
ptr set_memory(ptr dest, i32 value, usize size);

void console_write(cstr message, u8 color);
void console_write_error(cstr message, u8 color);

f64 get_absolute_time();

void sleep(u64 ms);

} // namespace ns::platform

#endif // PLATFORM_HEADER_INCLUDED
