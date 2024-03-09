#ifndef GAME_HEADER_INCLUDED
#define GAME_HEADER_INCLUDED

#include <defines.h>
#include <game_types.h>
#include <math/ns_math.h>

struct game_state {
  f32 delta_time;
  ns::mat4 view;
  ns::vec3 camera_position;
  ns::vec3 camera_euler;
  bool camera_dirty;
};

bool game_initialize(game *game_inst);

bool game_update(game *game_inst, f32 delta_time);

bool game_render(game *game_inst, f32 delta_time);

void game_on_resize(game *game_inst, u32 width, u32 height);

#endif // GAME_HEADER_INCLUDED
