#ifndef GAME_TYPES_HEADER_INCLUDED
#define GAME_TYPES_HEADER_INCLUDED

#include "./core/application.h"

struct game {
  ns::application_config app_config;

  bool (*initialize)(game *game_inst);

  bool (*update)(game *game_inst, f32 delta_time);

  bool (*render)(game *game_inst, f32 delta_time);

  void (*on_resize)(game *game_inst, u32 width, u32 height);

  ptr state;

  ptr application_state;
};

#endif // GAME_TYPES_HEADER_INCLUDED
