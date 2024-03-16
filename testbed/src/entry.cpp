#include "./game.h"
#include <core/memory.h>

#include <entry.h>

bool create_game(game *out_game) {
  out_game->app_config.start_pos_x = 100;
  out_game->app_config.start_pos_y = 100;
  out_game->app_config.start_width = 1280;
  out_game->app_config.start_height = 720;
  out_game->app_config.name = "Test app";

  out_game->initialize = game_initialize;
  out_game->update = game_update;
  out_game->render = game_render;
  out_game->on_resize = game_on_resize;

  out_game->state = alloc(sizeof(game_state), ns::MemTag::GAME);
  out_game->application_state = 0;

  return true;
}
