#ifndef ENTRY_HEADER_INCLUDED
#define ENTRY_HEADER_INCLUDED

#include "./core/application.h"
#include "./core/logger.h"
#include "./game_types.h"

extern bool create_game(game *out_game);

int main(void) {
  game game_inst;
  if (!create_game(&game_inst)) {
    NS_FATAL("Could not create game!");
    return -1;
  }

  if (!game_inst.initialize || !game_inst.update || !game_inst.render ||
      !game_inst.on_resize) {
    NS_FATAL("The game's function pointers must be assigned!");
    return -2;
  }

  if (!application_create(&game_inst)) {
    NS_INFO("Application failed to create!");
    return 1;
  }

  if (!application_run()) {
    NS_INFO("Application did not shutdown gracefully!");
    return 2;
  }

  return 0;
}

#endif // ENTRY_HEADER_INCLUDED
