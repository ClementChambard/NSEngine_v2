#include "./event.h"

#include "../containers/vector.h"

struct registered_event {
  ptr listener;
  PFNONEVENT callback;
};

#define MAX_MESSAGE_CODES 4096

struct event_system_state {
  ns::vector<registered_event> registered[MAX_MESSAGE_CODES];
};

static event_system_state *state_ptr;

void event_system_initialize(usize *memory_requirement, ptr state) {
  *memory_requirement = sizeof(event_system_state);
  if (state == nullptr) {
    return;
  }
  ns::mem_zero(state, sizeof(event_system_state));
  state_ptr = reinterpret_cast<event_system_state *>(state);
}

void event_system_shutdown(ptr /*state*/) {
  if (!state_ptr)
    return;
  for (u16 i = 0; i < MAX_MESSAGE_CODES; i++) {
    ns::vector<registered_event>().swap(state_ptr->registered[i]);
  }
  state_ptr = nullptr;
}

bool event_register(u16 code, ptr listener, PFNONEVENT on_event) {
  if (!state_ptr) {
    return false;
  }

  for (usize i = 0; i < state_ptr->registered[code].size(); i++) {
    if (state_ptr->registered[code][i].listener == listener) {
      // TODO(ClementChambard): warn
      return false;
    }
  }

  state_ptr->registered[code].push_back({
      .listener = listener,
      .callback = on_event,
  });

  return true;
}

bool event_unregister(u16 code, ptr listener, PFNONEVENT on_event) {
  if (!state_ptr) {
    return false;
  }

  for (auto it = state_ptr->registered[code].begin();
       it != state_ptr->registered[code].end(); it++) {
    if (it->listener == listener && it->callback == on_event) {
      state_ptr->registered[code].erase(it);
      return true;
    }
  }

  return false;
}

bool event_fire(u16 code, ptr sender, event_context context) {
  if (!state_ptr) {
    return false;
  }

  for (auto const &e : state_ptr->registered[code]) {
    if (e.callback(code, sender, e.listener, context)) {
      return true;
    }
  }

  return false;
}
