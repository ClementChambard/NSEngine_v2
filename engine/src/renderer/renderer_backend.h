#ifndef RENDERER_BACKEND_HEADER_INCLUDED
#define RENDERER_BACKEND_HEADER_INCLUDED

#include "./renderer_types.inl"

struct platform_state;

namespace ns {

bool renderer_backend_create(renderer_backend_type type,
                             renderer_backend *out_renderer_backend);
void renderer_backend_destroy(renderer_backend *renderer_backend);

} // namespace ns

#endif // RENDERER_BACKEND_HEADER_INCLUDED
