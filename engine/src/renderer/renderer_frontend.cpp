#include "./renderer_frontend.h"
#include "./renderer_backend.h"

#include "../core/logger.h"

#include "../math/ns_math.h"

#include "../resources/resource_types.h"

// TODO(ClementChambard): Temp
#include "../core/event.h"
#include "../systems/texture_system.h"
// TODO(ClementChambard): EndTemp

namespace ns {

struct renderer_system_state {
  renderer_backend backend;
  mat4 projection;
  mat4 view;
  f32 near_clip;
  f32 far_clip;

  // TODO(ClementChambard): Temp
  Texture *test_diffuse;
  // TODO(ClementChambard): EndTemp
};

static renderer_system_state *state_ptr = nullptr;

// TODO(ClementChambard): Temp
bool event_on_debug_event(u16, ptr, ptr, event_context) {
  cstr names[3] = {
      "Brick_01",
      "Brick_02",
      "Brick_03",
  };
  static i8 choice = 2;
  static bool first = true;

  cstr old_name = names[choice];

  choice++;
  choice %= 3;

  state_ptr->test_diffuse = texture_system_acquire(names[choice], false);
  if (!first) {
    texture_system_release(old_name);
  }
  first = false;
  return true;
}
// TODO(ClementChambard): EndTemp

bool renderer_system_initialize(usize *memory_requirement, ptr state,
                                cstr application_name) {
  *memory_requirement = sizeof(renderer_system_state);
  if (state == nullptr) {
    return true;
  }
  state_ptr = reinterpret_cast<renderer_system_state *>(state);

  // TODO(ClementChambard): Temp
  event_register(EVENT_CODE_DEBUG0, state_ptr, event_on_debug_event);
  state_ptr->test_diffuse = nullptr;
  // TODO(ClementChambard): EndTemp

  // TODO(ClementChambard): make it configurable
  renderer_backend_create(RENDERER_BACKEND_TYPE_VULKAN, &state_ptr->backend);
  state_ptr->backend.frame_number = 0;

  if (!state_ptr->backend.initialize(&state_ptr->backend, application_name)) {
    NS_FATAL("Renderer backend failed to initialize. Shutting down.");
    return false;
  }

  state_ptr->near_clip = 0.01f;
  state_ptr->far_clip = 1000.0f;
  state_ptr->projection =
      mat4::perspective(PI_1_4<float>, 1280.0f / 720.0f, state_ptr->near_clip,
                        state_ptr->far_clip);

  state_ptr->view = mat4::mk_translate({0.0f, 0.0f, -30.0f});

  return true;
}

void renderer_system_shutdown(ptr /*state*/) {
  // TODO(ClementChambard): Temp
  event_unregister(EVENT_CODE_DEBUG0, state_ptr, event_on_debug_event);
  // TODO(ClementChambard): EndTemp

  if (!state_ptr) {
    return;
  }
  state_ptr->backend.shutdown(&state_ptr->backend);
  state_ptr = nullptr;
}

void renderer_on_resized(u16 width, u16 height) {
  if (state_ptr) {
    state_ptr->backend.resized(&state_ptr->backend, width, height);
    state_ptr->projection = mat4::perspective(
        PI_1_4<float>, static_cast<f32>(width) / static_cast<f32>(height),
        state_ptr->near_clip, state_ptr->far_clip);
  } else {
    NS_WARN("renderer backend does not exist to accept resize: %i %i", width,
            height);
  }
}

bool renderer_begin_frame(f32 delta_time) {
  if (!state_ptr) {
    return false;
  }
  return state_ptr->backend.begin_frame(&state_ptr->backend, delta_time);
}

bool renderer_end_frame(f32 delta_time) {
  if (!state_ptr) {
    return false;
  }
  bool result = state_ptr->backend.end_frame(&state_ptr->backend, delta_time);
  state_ptr->backend.frame_number++;
  return result;
}

bool renderer_draw_frame(render_packet *packet) {
  if (renderer_begin_frame(packet->delta_time)) {
    state_ptr->backend.update_global_state(
        state_ptr->projection, state_ptr->view, vec3::zero(), vec4::one(), 0);

    geometry_render_data data{};
    data.object_id = 0;
    data.model = mat4(1.0f);

    if (!state_ptr->test_diffuse) {
      state_ptr->test_diffuse = texture_system_get_default_texture();
    }

    data.textures[0] = state_ptr->test_diffuse;
    state_ptr->backend.update_object(data);

    bool result = renderer_end_frame(packet->delta_time);
    if (!result) {
      NS_ERROR("renderer_end_frame failed. Application shutting down...");
      return false;
    }
  }

  return true;
}

void renderer_set_view(mat4 view) { state_ptr->view = view; }

void renderer_create_texture(cstr name, i32 width, i32 height,
                             i32 channel_count, robytes pixels,
                             bool has_transparency, Texture *out_texture) {
  state_ptr->backend.create_texture(name, width, height, channel_count, pixels,
                                    has_transparency, out_texture);
}

void renderer_destroy_texture(Texture *texture) {
  state_ptr->backend.destroy_texture(texture);
}

} // namespace ns
