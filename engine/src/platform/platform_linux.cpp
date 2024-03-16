#include "./platform.h"

#if NS_PLATFORM_LINUX

#include "../containers/vec.h"
#include "../core/event.h"
#include "../core/input.h"
#include "../core/logger.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <X11/XKBlib.h>
#include <X11/Xlib-xcb.h>
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <sys/time.h>
#include <xcb/xcb.h>

// for surface creation
#define VK_USE_PLATFORM_XCB_KHR
#include "../renderer/vulkan/vulkan_types.inl"
#include <vulkan/vulkan.h>

#if _POSIX_C_SOURCE >= 199309L
#include <time.h>
#else
#include <unistd.h>
#endif

namespace ns::platform {

struct platform_state {
  Display *display;
  xcb_connection_t *connection;
  xcb_window_t window;
  xcb_screen_t *screen;
  xcb_atom_t wm_protocols;
  xcb_atom_t wm_delete_win;
  VkSurfaceKHR surface;
};

static platform_state *state_ptr;

ns_key translate_keycode(u32 x_keycode);

bool startup(usize *memory_requirement, ptr state, cstr application_name, i32 x,
             i32 y, i32 width, i32 height) {
  *memory_requirement = sizeof(platform_state);
  if (state == nullptr) {
    return true;
  }
  state_ptr = reinterpret_cast<platform_state *>(state);

  state_ptr->display = XOpenDisplay(NULL);

  XAutoRepeatOff(state_ptr->display);

  state_ptr->connection = XGetXCBConnection(state_ptr->display);

  if (xcb_connection_has_error(state_ptr->connection)) {
    NS_FATAL("Failed to connect to X server via XCB.");
    return false;
  }

  const xcb_setup_t *setup = xcb_get_setup(state_ptr->connection);

  xcb_screen_iterator_t it = xcb_setup_roots_iterator(setup);
  int screen_p = 0;
  for (i32 s = screen_p; s > 0; s--) {
    xcb_screen_next(&it);
  }

  state_ptr->screen = it.data;

  state_ptr->window = xcb_generate_id(state_ptr->connection);

  u32 event_mask = XCB_CW_BACK_PIXEL | XCB_CW_EVENT_MASK;

  u32 event_values = XCB_EVENT_MASK_BUTTON_PRESS |
                     XCB_EVENT_MASK_BUTTON_RELEASE | XCB_EVENT_MASK_KEY_PRESS |
                     XCB_EVENT_MASK_KEY_RELEASE | XCB_EVENT_MASK_EXPOSURE |
                     XCB_EVENT_MASK_POINTER_MOTION |
                     XCB_EVENT_MASK_STRUCTURE_NOTIFY;

  u32 value_list[] = {state_ptr->screen->black_pixel, event_values};

  xcb_void_cookie_t cookie [[maybe_unused]] =
      xcb_create_window(state_ptr->connection, XCB_COPY_FROM_PARENT,
                        state_ptr->window, state_ptr->screen->root, x, y, width,
                        height, 0, XCB_WINDOW_CLASS_INPUT_OUTPUT,
                        state_ptr->screen->root_visual, event_mask, value_list);

  xcb_change_property(state_ptr->connection, XCB_PROP_MODE_REPLACE,
                      state_ptr->window, XCB_ATOM_WM_NAME, XCB_ATOM_STRING, 8,
                      std::strlen(application_name), application_name);

  xcb_intern_atom_cookie_t wm_delete_cookie =
      xcb_intern_atom(state_ptr->connection, 0, std::strlen("WM_DELETE_WINDOW"),
                      "WM_DELETE_WINDOW");
  xcb_intern_atom_cookie_t wm_protocols_cookie = xcb_intern_atom(
      state_ptr->connection, 0, std::strlen("WM_PROTOCOLS"), "WM_PROTOCOLS");
  xcb_intern_atom_reply_t *wm_delete_reply =
      xcb_intern_atom_reply(state_ptr->connection, wm_delete_cookie, NULL);
  xcb_intern_atom_reply_t *wm_protocols_reply =
      xcb_intern_atom_reply(state_ptr->connection, wm_protocols_cookie, NULL);
  state_ptr->wm_delete_win = wm_delete_reply->atom;
  state_ptr->wm_protocols = wm_protocols_reply->atom;

  xcb_change_property(state_ptr->connection, XCB_PROP_MODE_REPLACE,
                      state_ptr->window, wm_protocols_reply->atom, 4, 32, 1,
                      &wm_delete_reply->atom);

  xcb_map_window(state_ptr->connection, state_ptr->window);

  i32 stream_result = xcb_flush(state_ptr->connection);
  if (stream_result <= 0) {
    NS_FATAL("An error occurred when flushing the stream: %d", stream_result);
    return false;
  }

  return true;
}

void shutdown(ptr /*state*/) {
  if (!state_ptr)
    return;
  XAutoRepeatOn(state_ptr->display);

  xcb_destroy_window(state_ptr->connection, state_ptr->window);
}

bool pump_messages() {
  if (!state_ptr)
    return true;
  xcb_generic_event_t *event;

  bool quit_flagged = false;

  while (true) {
    event = xcb_poll_for_event(state_ptr->connection);
    if (event == 0) {
      break;
    }

    switch (event->response_type & ~0x80) {
    case XCB_KEY_PRESS:
    case XCB_KEY_RELEASE: {
      xcb_key_press_event_t *kb_event =
          reinterpret_cast<xcb_key_press_event_t *>(event);
      xcb_keycode_t code = kb_event->detail;
      KeySym key_sym =
          XkbKeycodeToKeysym(state_ptr->display, static_cast<KeyCode>(code), 0,
                             code & ShiftMask ? 1 : 0);

      InputManager::process_key(translate_keycode(key_sym),
                                event->response_type == XCB_KEY_PRESS);
    } break;
    case XCB_BUTTON_PRESS:
    case XCB_BUTTON_RELEASE: {
      xcb_button_press_event_t *mouse_event =
          reinterpret_cast<xcb_button_press_event_t *>(event);
      switch (mouse_event->detail) {
      case XCB_BUTTON_INDEX_1:
        InputManager::process_button(NSB_LEFT,
                                     event->response_type == XCB_BUTTON_PRESS);
        break;
      case XCB_BUTTON_INDEX_2:
        InputManager::process_button(NSB_MIDDLE,
                                     event->response_type == XCB_BUTTON_PRESS);
        break;
      case XCB_BUTTON_INDEX_3:
        InputManager::process_button(NSB_RIGHT,
                                     event->response_type == XCB_BUTTON_PRESS);
        break;
      }
    } break;
    case XCB_MOTION_NOTIFY: {
      xcb_motion_notify_event_t *mouse_event =
          reinterpret_cast<xcb_motion_notify_event_t *>(event);
      InputManager::process_mouse_move(mouse_event->event_x,
                                       mouse_event->event_y);
    } break;
    case XCB_CONFIGURE_NOTIFY: {
      xcb_configure_notify_event_t *configure_event =
          reinterpret_cast<xcb_configure_notify_event_t *>(event);

      event_context context;
      context.data.U16[0] = configure_event->width;
      context.data.U16[1] = configure_event->height;
      event_fire(EVENT_CODE_RESIZED, nullptr, context);
    } break;
    case XCB_CLIENT_MESSAGE: {
      xcb_client_message_event_t *cm =
          reinterpret_cast<xcb_client_message_event_t *>(event);

      if (cm->data.data32[0] == state_ptr->wm_delete_win) {
        quit_flagged = true;
      }
    } break;
    default:
      break;
    }

    std::free(event);
  }
  return !quit_flagged;
}

ptr allocate_memory(usize size, bool /* aligned */) {
  return std::malloc(size);
}

ptr reallocate_memory(ptr block, usize new_size, bool /* aligned */) {
  return std::realloc(block, new_size);
}

void free_memory(ptr block, bool /* aligned */) { std::free(block); }

ptr zero_memory(ptr block, usize size) { return std::memset(block, 0, size); }

ptr copy_memory(ptr dest, roptr source, usize size) {
  return std::memcpy(dest, source, size);
}

ptr set_memory(ptr dest, i32 value, usize size) {
  return std::memset(dest, value, size);
}

void console_write(cstr message, u8 color) {
  cstr levels[6] = {"0;41", "1;31", "1;33", "1;32", "1;34", "0;36"};
  std::printf("\033[%sm%s\033[0m", levels[color], message);
}

void console_write_error(cstr message, u8 color) {
  cstr levels[6] = {"0;41", "1;31", "1;33", "1;32", "1;34", "0;36"};
  std::printf("\033[%sm%s\033[0m", levels[color], message);
}

f64 get_absolute_time() {
  timespec now;
  clock_gettime(CLOCK_MONOTONIC, &now);
  return now.tv_sec + now.tv_nsec * 0.000000001;
}

void sleep(u64 ms) {
#if _POSIX_C_SOURCE >= 199309L
  timespec ts;
  ts.tv_sec = ms / 1000;
  ts.tv_nsec = (ms % 1000) * 1000 * 1000;
  nanosleep(&ts, 0);
#else
  if (ms >= 1000) {
    sleep(ms / 1000);
  }
  usleep((ms % 1000) * 1000);
#endif
}

void get_required_extension_names(Vec<cstr> *names_darray) {
  names_darray->push("VK_KHR_xcb_surface");
}

bool create_vulkan_surface(vulkan::Context *context) {
  if (!state_ptr)
    return false;
  VkXcbSurfaceCreateInfoKHR create_info{};
  create_info.sType = VK_STRUCTURE_TYPE_XCB_SURFACE_CREATE_INFO_KHR;
  create_info.connection = state_ptr->connection;
  create_info.window = state_ptr->window;

  VkResult result = vkCreateXcbSurfaceKHR(
      context->instance, &create_info, context->allocator, &state_ptr->surface);
  if (result != VK_SUCCESS) {
    NS_FATAL("Vulkan surface creation failed.");
    return false;
  }

  context->surface = state_ptr->surface;
  return true;
}

ns_key translate_keycode(u32 x_keycode) {
  switch (x_keycode) {
  case XK_BackSpace:
    return NSK_BACKSPACE;
  case XK_Return:
    return NSK_RETURN;
  case XK_Tab:
    return NSK_TAB;
  case XK_Pause:
    return NSK_PAUSE;
  case XK_Caps_Lock:
    return NSK_CAPITAL;
  case XK_Escape:
    return NSK_ESCAPE;
  case XK_Mode_switch:
    return NSK_MODECHANGE;
  case XK_space:
    return NSK_SPACE;
  case XK_Prior:
    return NSK_PRIOR;
  case XK_Next:
    return NSK_NEXT;
  case XK_End:
    return NSK_END;
  case XK_Home:
    return NSK_HOME;
  case XK_Left:
    return NSK_LEFT;
  case XK_Up:
    return NSK_UP;
  case XK_Right:
    return NSK_RIGHT;
  case XK_Down:
    return NSK_DOWN;
  case XK_Select:
    return NSK_SELECT;
  case XK_Print:
    return NSK_PRINT;
  case XK_Execute:
    return NSK_EXECUTE;
  case XK_Insert:
    return NSK_INSERT;
  case XK_Delete:
    return NSK_DELETE;
  case XK_Help:
    return NSK_HELP;
  case XK_Meta_L:
    return NSK_LWIN;
  case XK_Meta_R:
    return NSK_RWIN;
  case XK_KP_0:
    return NSK_NUMPAD0;
  case XK_KP_1:
    return NSK_NUMPAD1;
  case XK_KP_2:
    return NSK_NUMPAD2;
  case XK_KP_3:
    return NSK_NUMPAD3;
  case XK_KP_4:
    return NSK_NUMPAD4;
  case XK_KP_5:
    return NSK_NUMPAD5;
  case XK_KP_6:
    return NSK_NUMPAD6;
  case XK_KP_7:
    return NSK_NUMPAD7;
  case XK_KP_8:
    return NSK_NUMPAD8;
  case XK_KP_9:
    return NSK_NUMPAD9;
  case XK_multiply:
    return NSK_MULTIPLY;
  case XK_KP_Add:
    return NSK_ADD;
  case XK_KP_Separator:
    return NSK_SEPARATOR;
  case XK_KP_Subtract:
    return NSK_SUBTRACT;
  case XK_KP_Decimal:
    return NSK_DECIMAL;
  case XK_KP_Divide:
    return NSK_DIVIDE;
  case XK_F1:
    return NSK_F1;
  case XK_F2:
    return NSK_F2;
  case XK_F3:
    return NSK_F3;
  case XK_F4:
    return NSK_F4;
  case XK_F5:
    return NSK_F5;
  case XK_F6:
    return NSK_F6;
  case XK_F7:
    return NSK_F7;
  case XK_F8:
    return NSK_F8;
  case XK_F9:
    return NSK_F9;
  case XK_F10:
    return NSK_F10;
  case XK_F11:
    return NSK_F11;
  case XK_F12:
    return NSK_F12;
  case XK_F13:
    return NSK_F13;
  case XK_F14:
    return NSK_F14;
  case XK_F15:
    return NSK_F15;
  case XK_F16:
    return NSK_F16;
  case XK_F17:
    return NSK_F17;
  case XK_F18:
    return NSK_F18;
  case XK_F19:
    return NSK_F19;
  case XK_F20:
    return NSK_F20;
  case XK_F21:
    return NSK_F21;
  case XK_F22:
    return NSK_F22;
  case XK_F23:
    return NSK_F23;
  case XK_F24:
    return NSK_F24;
  case XK_Num_Lock:
    return NSK_NUMLOCK;
  case XK_Scroll_Lock:
    return NSK_SCROLL;
  case XK_KP_Equal:
    return NSK_NUMPAD_EQUAL;
  case XK_Shift_L:
    return NSK_LSHIFT;
  case XK_Shift_R:
    return NSK_RSHIFT;
  case XK_Control_L:
    return NSK_LCONTROL;
  case XK_Control_R:
    return NSK_RCONTROL;
  case XK_Alt_L:
    return NSK_LALT;
  case XK_Alt_R:
    return NSK_RALT;
  case XK_semicolon:
    return NSK_SEMICOLON;
  case XK_plus:
    return NSK_PLUS;
  case XK_comma:
    return NSK_COMMA;
  case XK_minus:
    return NSK_MINUS;
  case XK_period:
    return NSK_PERIOD;
  case XK_slash:
    return NSK_SLASH;
  case XK_grave:
    return NSK_GRAVE;
  case XK_a:
  case XK_A:
    return NSK_A;
  case XK_b:
  case XK_B:
    return NSK_B;
  case XK_c:
  case XK_C:
    return NSK_C;
  case XK_d:
  case XK_D:
    return NSK_D;
  case XK_e:
  case XK_E:
    return NSK_E;
  case XK_f:
  case XK_F:
    return NSK_F;
  case XK_g:
  case XK_G:
    return NSK_G;
  case XK_h:
  case XK_H:
    return NSK_H;
  case XK_i:
  case XK_I:
    return NSK_I;
  case XK_j:
  case XK_J:
    return NSK_J;
  case XK_k:
  case XK_K:
    return NSK_K;
  case XK_l:
  case XK_L:
    return NSK_L;
  case XK_m:
  case XK_M:
    return NSK_M;
  case XK_n:
  case XK_N:
    return NSK_N;
  case XK_o:
  case XK_O:
    return NSK_O;
  case XK_p:
  case XK_P:
    return NSK_P;
  case XK_q:
  case XK_Q:
    return NSK_Q;
  case XK_r:
  case XK_R:
    return NSK_R;
  case XK_s:
  case XK_S:
    return NSK_S;
  case XK_t:
  case XK_T:
    return NSK_T;
  case XK_u:
  case XK_U:
    return NSK_U;
  case XK_v:
  case XK_V:
    return NSK_V;
  case XK_w:
  case XK_W:
    return NSK_W;
  case XK_x:
  case XK_X:
    return NSK_X;
  case XK_y:
  case XK_Y:
    return NSK_Y;
  case XK_z:
  case XK_Z:
    return NSK_Z;
  default:
    return static_cast<ns_key>(0);
  }
}

} // namespace ns::platform

#endif // NS_PLATFORM_LINUX
