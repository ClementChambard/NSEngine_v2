#ifndef APPLICATION_HEADER_INCLUDED
#define APPLICATION_HEADER_INCLUDED

#include "../defines.h"

struct game;

namespace ns {

struct application_config {
  i16 start_pos_x;
  i16 start_pos_y;
  i16 start_width;
  i16 start_height;
  cstr name;
};

NS_API bool application_create(game *game_inst);

NS_API bool application_run();

void application_get_framebuffer_size(u32 *width, u32 *height);

} // namespace ns

#endif // APPLICATION_HEADER_INCLUDED
