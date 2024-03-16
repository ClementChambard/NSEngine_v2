#ifndef LOADER_UTILS_HEADER_INCLUDED
#define LOADER_UTILS_HEADER_INCLUDED

#include "../resource_types.h"

namespace ns {

struct resource_loader;

void resource_unload(resource_loader *self, Resource *resource, MemTag tag);

} // namespace ns

#endif // LOADER_UTILS_HEADER_INCLUDED
