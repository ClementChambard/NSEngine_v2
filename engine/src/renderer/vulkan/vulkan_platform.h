#ifndef VULKAN_PLATFORM_HEADER_INCLUDED
#define VULKAN_PLATFORM_HEADER_INCLUDED

#include "../../containers/vector.h"
#include "../../defines.h"

namespace ns::vulkan {
struct Context;
}

void platform_get_required_extension_names(ns::vector<cstr> *names_darray);

bool platform_create_vulkan_surface(ns::vulkan::Context *context);

#endif // VULKAN_PLATFORM_HEADER_INCLUDED
