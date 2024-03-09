#include "./input.h"
#include "./event.h"
#include "./logger.h"
#include "./ns_memory.h"

namespace ns {

static InputManager *instance = nullptr;

void InputManager::initialize(usize *memory_requirements, ptr state) {
  *memory_requirements = sizeof(InputManager);
  if (state == nullptr) {
    return;
  }
  mem_zero(state, sizeof(InputManager));
  instance = reinterpret_cast<InputManager *>(state);
}
void InputManager::cleanup(ptr /*state*/) { instance = nullptr; }
void InputManager::update(f64 /* delta_time */) {
  if (!instance)
    return;
  mem_copy(instance->prev_keyboard, instance->keyboard,
           sizeof(instance->keyboard));
  mem_copy(instance->prev_mouse, instance->mouse, sizeof(instance->mouse));
  instance->prev_mouse_x = instance->mouse_x;
  instance->prev_mouse_y = instance->mouse_y;
}

void InputManager::process_key(ns_key key, bool pressed) {
  if (!instance)
    return;
  if (instance->keyboard[key] != pressed) {
    instance->keyboard[key] = pressed;

    event_context context;
    context.data.U16[0] = key;
    event_fire(pressed ? EVENT_CODE_KEY_PRESSED : EVENT_CODE_KEY_RELEASED,
               nullptr, context);
  }
}

void InputManager::process_button(ns_btn button, bool pressed) {
  if (!instance)
    return;
  if (instance->mouse[button] != pressed) {
    instance->mouse[button] = pressed;

    event_context context;
    context.data.U16[0] = button;
    event_fire(pressed ? EVENT_CODE_BUTTON_PRESSED : EVENT_CODE_BUTTON_RELEASED,
               nullptr, context);
  }
}

void InputManager::process_mouse_move(i16 x, i16 y) {
  if (!instance)
    return;
  if (instance->mouse_x != x || instance->mouse_y != y) {
    // NS_DEBUG("Mouse pos: %d, %d", x, y);

    instance->mouse_x = x;
    instance->mouse_y = y;

    event_context context;
    context.data.U16[0] = x;
    context.data.U16[1] = y;
    event_fire(EVENT_CODE_MOUSE_MOVED, nullptr, context);
  }
}

void InputManager::process_mouse_wheel(i8 z_delta) {
  event_context context;
  context.data.U8[0] = z_delta;
  event_fire(EVENT_CODE_MOUSE_WHEEL, nullptr, context);
}

inline bool InputManager::_k(usize i) { return keyboard[i]; }
inline bool InputManager::_pk(usize i) { return prev_keyboard[i]; }
inline bool InputManager::_m(usize i) { return mouse[i]; }
inline bool InputManager::_pm(usize i) { return prev_mouse[i]; }
inline void InputManager::_mp(i32 *x, i32 *y) {
  *x = mouse_x;
  *y = mouse_y;
}
inline void InputManager::_pmp(i32 *x, i32 *y) {
  *x = prev_mouse_x;
  *y = prev_mouse_y;
}
inline void InputManager::_mm(i32 *x, i32 *y) {
  *x = mouse_x - prev_mouse_x;
  *y = mouse_y - prev_mouse_y;
}

namespace keyboard {

NS_API bool pressed(ns_key key) {
  return instance->_k(key) && !instance->_pk(key);
}
NS_API bool released(ns_key key) {
  return !instance->_k(key) && instance->_pk(key);
}
NS_API bool down(ns_key key) { return instance->_k(key); }
NS_API bool up(ns_key key) { return !instance->_k(key); }
NS_API bool was_down(ns_key key) { return instance->_pk(key); }
NS_API bool was_up(ns_key key) { return !instance->_pk(key); }

} // namespace keyboard

namespace mouse {

NS_API bool pressed(ns_btn button) {
  return instance->_m(button) && !instance->_pm(button);
}
NS_API bool released(ns_btn button) {
  return !instance->_m(button) && instance->_pm(button);
}
NS_API bool down(ns_btn button) { return instance->_m(button); }
NS_API bool up(ns_btn button) { return !instance->_m(button); }
NS_API bool was_down(ns_btn button) { return instance->_pm(button); }
NS_API bool was_up(ns_btn button) { return !instance->_pm(button); }

NS_API void position(i32 *x, i32 *y) { instance->_mp(x, y); }
NS_API void prev_position(i32 *x, i32 *y) { instance->_pmp(x, y); }
NS_API void motion(i32 *x, i32 *y) { instance->_mm(x, y); }

} // namespace mouse
} // namespace ns
