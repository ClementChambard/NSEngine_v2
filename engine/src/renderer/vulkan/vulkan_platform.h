#ifndef VULKAN_PLATFORM_HEADER_INCLUDED
#define VULKAN_PLATFORM_HEADER_INCLUDED

#include "../../containers/vec.h"
#include "../../defines.h"

namespace ns::vulkan {
struct Context;
}

namespace ns::platform {

void get_required_extension_names(Vec<cstr> *names_darray);

bool create_vulkan_surface(vulkan::Context *context);

} // namespace ns::platform

#endif // VULKAN_PLATFORM_HEADER_INCLUDED
