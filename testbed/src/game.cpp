#include "./game.h"
#include <core/event.h>
#include <core/input.h>
#include <core/logger.h>
#include <core/memory.h>
#include <math/math.h>

// HACK: remove include when possible
#include <renderer/renderer_frontend.h>

void recalculate_view_matrix(game_state *state) {
  if (state->camera_dirty) {
    state->view = ns::mat4::mk_euler_xyz(state->camera_euler)
                      .translate(state->camera_position)
                      .inverse();
    state->camera_dirty = false;
  }
}

void camera_yaw(game_state *state, f32 amount) {
  state->camera_euler.y += amount;
  state->camera_dirty = true;
}

void camera_pitch(game_state *state, f32 amount) {
  state->camera_euler.x += amount;

  if (state->camera_euler.x > 89.0f) {
    state->camera_euler.x = 89.0f;
  }
  if (state->camera_euler.x < -89.0f) {
    state->camera_euler.x = -89.0f;
  }

  state->camera_dirty = true;
}

bool game_initialize(game *game_inst) {
  game_state *state = reinterpret_cast<game_state *>(game_inst->state);

  state->camera_position = {0.0f, 0.0f, 30.0f};
  state->camera_euler = {};
  state->camera_dirty = false;
  state->view = ns::mat4::mk_translate(state->camera_position).inverse();

  return true;
}

bool game_update(game *game_inst, f32 delta_time) {
  game_state *state = reinterpret_cast<game_state *>(game_inst->state);

  static u64 alloc_count = 0;
  u64 prev_alloc_count = alloc_count;
  alloc_count = ns::get_memory_alloc_count();
  if (ns::keyboard::pressed(NSK_M)) {
    NS_DEBUG("Allocations: %llu (%llu this frame)", alloc_count,
             alloc_count - prev_alloc_count);
  }

  // TODO(ClementChambard): Temp
  if (ns::keyboard::released(NSK_T)) {
    NS_DEBUG("Swapping texture!");
    ns::event_context ctx{};
    ns::event_fire(ns::EVENT_CODE_DEBUG0, game_inst, ctx);
  }
  // TODO(ClementChambard): EndTemp

  ns::vec3 vel{};
  float mv_speed = 20.0f * delta_time;

  if (ns::keyboard::down(NSK_LEFT)) {
    camera_yaw(state, 1.0f * delta_time);
  }
  if (ns::keyboard::down(NSK_RIGHT)) {
    camera_yaw(state, -1.0f * delta_time);
  }
  if (ns::keyboard::down(NSK_UP)) {
    camera_pitch(state, 1.0f * delta_time);
  }
  if (ns::keyboard::down(NSK_DOWN)) {
    camera_pitch(state, -1.0f * delta_time);
  }
  if (ns::keyboard::down(NSK_W)) {
    vel += state->view.forward();
  }
  if (ns::keyboard::down(NSK_S)) {
    vel += state->view.backward();
  }
  if (ns::keyboard::down(NSK_A)) {
    vel += state->view.left();
  }
  if (ns::keyboard::down(NSK_D)) {
    vel += state->view.right();
  }
  if (ns::keyboard::down(NSK_SPACE)) {
    vel += ns::vec3::up();
  }
  if (ns::keyboard::down(NSK_LSHIFT)) {
    vel += ns::vec3::down();
  }
  if (vel != ns::vec3{}) {
    state->camera_position += mv_speed * vel.normalized();
    state->camera_dirty = true;
  }

  recalculate_view_matrix(state);

  // HACK: remove this call when possible
  ns::renderer_set_view(state->view);

  return true;
}

bool game_render(game * /*game_inst*/, f32 /*delta_time*/) { return true; }

void game_on_resize(game * /*game_inst*/, u32 /*width*/, u32 /*height*/) {}
