#ifndef PLATFORM_HEADER_INCLUDED
#define PLATFORM_HEADER_INCLUDED

#include "../defines.h"

bool platform_system_startup(usize *memory_requirement, ptr state,
                             cstr application_name, i32 x, i32 y, i32 width,
                             i32 height);

void platform_system_shutdown(ptr state);

bool platform_pump_messages();

ptr platform_allocate(usize size, bool aligned);
void platform_free(ptr block, bool aligned);
ptr platform_zero_memory(ptr block, usize size);
ptr platform_copy_memory(ptr dest, roptr source, usize size);
ptr platform_set_memory(ptr dest, i32 value, usize size);

void platform_console_write(cstr message, u8 color);
void platform_console_write_error(cstr message, u8 color);

f64 platform_get_absolute_time();

void platform_sleep(u64 ms);

#endif // PLATFORM_HEADER_INCLUDED
