#include "./application.h"
#include "../game_types.h"
#include "../platform/platform.h"
#include "./clock.h"
#include "./event.h"
#include "./input.h"
#include "./logger.h"
#include "./ns_memory.h"
#include "./ns_string.h"

#include "../memory/linear_allocator.h"

#include "../renderer/renderer_frontend.h"

#include "../systems/geometry_system.h"
#include "../systems/material_system.h"
#include "../systems/resource_system.h"
#include "../systems/texture_system.h"

#include <new>

// temp
#include "../math/ns_math.h"

struct application_state {
  game *game_inst;
  bool is_running;
  bool is_suspended;
  i16 width;
  i16 height;
  ns::clock_t clock;
  f64 last_time;
  linear_allocator systems_allocator;

  u64 event_system_memory_requirement;
  ptr event_system_state;

  u64 memory_system_memory_requirement;
  ptr memory_system_state;

  u64 logging_system_memory_requirement;
  ptr logging_system_state;

  u64 input_system_memory_requirement;
  ptr input_system_state;

  u64 platform_system_memory_requirement;
  ptr platform_system_state;

  u64 resource_system_memory_requirement;
  ptr resource_system_state;

  u64 renderer_system_memory_requirement;
  ptr renderer_system_state;

  u64 texture_system_memory_requirement;
  ptr texture_system_state;

  u64 material_system_memory_requirement;
  ptr material_system_state;

  u64 geometry_system_memory_requirement;
  ptr geometry_system_state;

  // temp
  ns::Geometry *test_geometry;
};

static application_state *app_state;

bool application_on_event(u16 code, ptr sender, ptr listener_inst,
                          event_context context);
bool application_on_resized(u16 code, ptr sender, ptr listener_inst,
                            event_context context);

// temp
bool application_on_debug_event(u16, ptr, ptr, event_context) {
  cstr names[3] = {
      "Brick_01",
      "Brick_02",
      "Brick_03",
  };
  static i8 choice = 2;

  cstr old_name = names[choice];

  choice++;
  choice %= 3;

  static bool first = true;

  if (app_state->test_geometry) {
    app_state->test_geometry->material->diffuse_map.texture =
        ns::texture_system_acquire(names[choice], true);
    if (!app_state->test_geometry->material->diffuse_map.texture) {
      NS_WARN("Unable to load texture '%s' for test geometry, using default.",
              names[choice]);
      app_state->test_geometry->material->diffuse_map.texture =
          ns::texture_system_get_default_texture();
    }
    if (!first) {
      ns::texture_system_release(old_name);
    }
    first = false;
  }

  return true;
}

bool application_create(game *game_inst) {
  if (game_inst->application_state) {
    NS_ERROR("application_create called more than once");
    return false;
  }

  game_inst->application_state =
      ns::alloc(sizeof(application_state), ns::mem_tag::APPLICATION);
  app_state =
      reinterpret_cast<application_state *>(game_inst->application_state);
  app_state->game_inst = game_inst;

  usize systems_allocator_total_size = 64 * 1024 * 1024;

  new (&app_state->systems_allocator)
      linear_allocator(systems_allocator_total_size, nullptr);

  // initialize subsystems
  ns::memory_system_initialize(&app_state->memory_system_memory_requirement,
                               nullptr);
  app_state->memory_system_state = app_state->systems_allocator.allocate(
      app_state->memory_system_memory_requirement);
  ns::memory_system_initialize(&app_state->memory_system_memory_requirement,
                               app_state->memory_system_state);

  event_system_initialize(&app_state->event_system_memory_requirement, nullptr);
  app_state->event_system_state = app_state->systems_allocator.allocate(
      app_state->event_system_memory_requirement);
  event_system_initialize(&app_state->event_system_memory_requirement,
                          app_state->event_system_state);

  ns::initialize_logging(&app_state->logging_system_memory_requirement,
                         nullptr);
  app_state->logging_system_state = app_state->systems_allocator.allocate(
      app_state->logging_system_memory_requirement);
  if (!ns::initialize_logging(&app_state->logging_system_memory_requirement,
                              app_state->logging_system_state)) {
    NS_ERROR("Failed to initialize logging system; shutting down.");
    return false;
  }
  ns::InputManager::initialize(&app_state->input_system_memory_requirement,
                               nullptr);
  app_state->input_system_state = app_state->systems_allocator.allocate(
      app_state->input_system_memory_requirement);
  ns::InputManager::initialize(&app_state->input_system_memory_requirement,
                               app_state->input_system_state);

  app_state->is_running = true;
  app_state->is_suspended = false;

  event_register(EVENT_CODE_APPLICATION_QUIT, nullptr, application_on_event);
  event_register(EVENT_CODE_RESIZED, nullptr, application_on_resized);
  // temp
  event_register(EVENT_CODE_DEBUG0, nullptr, application_on_debug_event);

  platform_system_startup(&app_state->platform_system_memory_requirement,
                          nullptr, nullptr, 0, 0, 0, 0);
  app_state->platform_system_state = app_state->systems_allocator.allocate(
      app_state->platform_system_memory_requirement);
  if (!platform_system_startup(
          &app_state->platform_system_memory_requirement,
          app_state->platform_system_state, game_inst->app_config.name,
          game_inst->app_config.start_pos_x, game_inst->app_config.start_pos_y,
          game_inst->app_config.start_width,
          game_inst->app_config.start_height)) {
    return false;
  }

  ns::resource_system_config resource_sys_cfg{32, "../assets"};
  ns::resource_system_initialize(&app_state->resource_system_memory_requirement,
                                 nullptr, resource_sys_cfg);
  app_state->resource_system_state = app_state->systems_allocator.allocate(
      app_state->resource_system_memory_requirement);
  if (!ns::resource_system_initialize(
          &app_state->resource_system_memory_requirement,
          app_state->resource_system_state, resource_sys_cfg)) {
    NS_FATAL("Failed to initialize resource system. Aborting application.");
    return false;
  }

  ns::renderer_system_initialize(&app_state->renderer_system_memory_requirement,
                                 nullptr, nullptr);
  app_state->renderer_system_state = app_state->systems_allocator.allocate(
      app_state->renderer_system_memory_requirement);
  if (!ns::renderer_system_initialize(
          &app_state->renderer_system_memory_requirement,
          app_state->renderer_system_state, game_inst->app_config.name)) {
    NS_FATAL("Failed to initialize renderer. Aborting application.");
    return false;
  }

  ns::texture_system_config texture_sys_cfg{65536};
  ns::texture_system_initialize(&app_state->texture_system_memory_requirement,
                                nullptr, texture_sys_cfg);
  app_state->texture_system_state = app_state->systems_allocator.allocate(
      app_state->texture_system_memory_requirement);
  if (!ns::texture_system_initialize(
          &app_state->texture_system_memory_requirement,
          app_state->texture_system_state, texture_sys_cfg)) {
    NS_FATAL("Failed to initialize texture system. Aborting application.");
    return false;
  }

  ns::material_system_config material_sys_cfg{4096};
  ns::material_system_initialize(&app_state->material_system_memory_requirement,
                                 nullptr, material_sys_cfg);
  app_state->material_system_state = app_state->systems_allocator.allocate(
      app_state->material_system_memory_requirement);
  if (!ns::material_system_initialize(
          &app_state->material_system_memory_requirement,
          app_state->material_system_state, material_sys_cfg)) {
    NS_FATAL("Failed to initialize material system. Aborting application.");
    return false;
  }

  ns::geometry_system_config geometry_sys_cfg{4096};
  ns::geometry_system_initialize(&app_state->geometry_system_memory_requirement,
                                 nullptr, geometry_sys_cfg);
  app_state->geometry_system_state = app_state->systems_allocator.allocate(
      app_state->geometry_system_memory_requirement);
  if (!ns::geometry_system_initialize(
          &app_state->geometry_system_memory_requirement,
          app_state->geometry_system_state, geometry_sys_cfg)) {
    NS_FATAL("Failed to initialize geometry system. Aborting application.");
    return false;
  }

  // temp
  // app_state->test_geometry = ns::geometry_system_get_default();
  ns::geometry_config config = ns::geometry_config::plane(
      10.0f, 10.0f, 5, 5, 2.0f, 2.0f, "test geometry", "test_material");
  app_state->test_geometry = ns::geometry_system_acquire(config, true);

  ns::free(config.vertices, sizeof(ns::vertex_3d) * config.vertex_count,
           ns::mem_tag::ARRAY);
  ns::free(config.indices, sizeof(u32) * config.index_count,
           ns::mem_tag::ARRAY);

  if (!app_state->game_inst->initialize(app_state->game_inst)) {
    NS_FATAL("Game failed to initialize.");
    return false;
  }

  app_state->game_inst->on_resize(app_state->game_inst, app_state->width,
                                  app_state->height);
  return true;
}

bool application_run() {
  app_state->clock.start();
  app_state->clock.update();
  app_state->last_time = app_state->clock.elapsed;
  f64 running_time [[maybe_unused]] = 0.0;
  u8 frame_count [[maybe_unused]] = 0;
  f64 target_frame_seconds = 1.0 / 60.0;

  NS_TRACE(ns::get_memory_usage_str());

  while (app_state->is_running) {
    if (!platform_pump_messages()) {
      app_state->is_running = false;
    }

    if (app_state->is_suspended)
      continue;

    app_state->clock.update();
    f64 current_time = app_state->clock.elapsed;
    f64 delta = (current_time - app_state->last_time);
    f64 frame_start_time = platform_get_absolute_time();

    if (!app_state->game_inst->update(app_state->game_inst,
                                      static_cast<f32>(delta))) {
      NS_FATAL("Game update failed, shutting down.");
      app_state->is_running = false;
      break;
    }

    if (!app_state->game_inst->render(app_state->game_inst,
                                      static_cast<f32>(delta))) {
      NS_FATAL("Game render failed, shutting down.");
      app_state->is_running = false;
      break;
    }

    // TODO(ClementChambard): refactor packet creation
    ns::render_packet packet;
    packet.delta_time = delta;

    ns::geometry_render_data test_render;
    test_render.geometry = app_state->test_geometry;
    test_render.model = ns::mat4(1.0f);
    packet.geometry_count = 1;
    packet.geometries = &test_render;

    renderer_draw_frame(&packet);

    f64 frame_end_time = platform_get_absolute_time();
    f64 frame_elapsed_time = frame_end_time - frame_start_time;
    running_time += frame_elapsed_time;
    f64 remaining_seconds = target_frame_seconds - frame_elapsed_time;

    if (remaining_seconds > 0.0) {
      u64 remaining_ms = (remaining_seconds * 1000);

      bool limit_frames = false;
      if (remaining_ms > 0 && limit_frames) {
        platform_sleep(remaining_ms - 1);
      }

      frame_count++;
    }

    ns::InputManager::update(delta);

    app_state->last_time = current_time;
  }

  // temp
  event_unregister(EVENT_CODE_DEBUG0, nullptr, application_on_debug_event);
  event_unregister(EVENT_CODE_APPLICATION_QUIT, nullptr, application_on_event);
  event_unregister(EVENT_CODE_RESIZED, nullptr, application_on_resized);

  ns::InputManager::cleanup(app_state->input_system_state);

  ns::geometry_system_shutdown(app_state->geometry_system_state);

  ns::material_system_shutdown(app_state->material_system_state);

  ns::texture_system_shutdown(app_state->texture_system_state);

  ns::renderer_system_shutdown(app_state->renderer_system_state);

  ns::resource_system_shutdown(app_state->resource_system_state);

  platform_system_shutdown(app_state->platform_system_state);

  ns::shutdown_logging(app_state->logging_system_state); // ???

  event_system_shutdown(app_state->event_system_state);

  NS_TRACE(ns::get_memory_usage_str());

  ns::memory_system_shutdown(app_state->memory_system_state);

  return true;
}

void application_get_framebuffer_size(u32 *width, u32 *height) {
  *width = app_state->width;
  *height = app_state->height;
}

bool application_on_event(u16 code, ptr, ptr, event_context) {
  switch (code) {
  case EVENT_CODE_APPLICATION_QUIT: {
    NS_INFO("EVENT_CODE_APPLICATION_QUIT recieved, shutting down.");
    app_state->is_running = false;
    return true;
  }
  }

  return false;
}

bool application_on_resized(u16 code, ptr, ptr, event_context context) {
  if (code != EVENT_CODE_RESIZED)
    return false;
  u16 width = context.data.U16[0];
  u16 height = context.data.U16[1];

  if (width == app_state->width && height == app_state->height)
    return false;
  app_state->width = width;
  app_state->height = height;

  NS_DEBUG("Window resize: %i, %i", width, height);

  if (width == 0 || height == 0) {
    NS_INFO("Window minimized, suspending application.");
    app_state->is_suspended = true;
    return true;
  }
  if (app_state->is_suspended) {
    NS_INFO("Window restored, resuming application.");
    app_state->is_suspended = false;
  }
  app_state->game_inst->on_resize(app_state->game_inst, width, height);
  ns::renderer_on_resized(width, height);

  return false;
}
