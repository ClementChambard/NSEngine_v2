#ifndef EVENT_HEADER_INCLUDED
#define EVENT_HEADER_INCLUDED

#include "../defines.h"

struct event_context {
  union {
    i64 I64[2];
    u64 U64[2];
    f64 F64[2];

    i32 I32[4];
    u32 U32[4];
    f32 F32[4];

    i16 I16[8];
    u16 U16[8];

    i8 I8[16];
    u8 U8[86];

    char c[16];
  } data;
};

typedef bool (*PFNONEVENT)(u16 code, ptr sender, ptr listener_inst,
                           event_context data);

void event_system_initialize(usize *memory_requirement, ptr state);
void event_system_shutdown(ptr state);

NS_API bool event_register(u16 code, ptr listener, PFNONEVENT on_event);

NS_API bool event_unregister(u16 code, ptr listener, PFNONEVENT on_event);

NS_API bool event_fire(u16 code, ptr sender, event_context context);

enum system_event_code {
  EVENT_CODE_APPLICATION_QUIT = 0x01,
  EVENT_CODE_KEY_PRESSED = 0x02,
  EVENT_CODE_KEY_RELEASED = 0x03,
  EVENT_CODE_BUTTON_PRESSED = 0x04,
  EVENT_CODE_BUTTON_RELEASED = 0x05,
  EVENT_CODE_MOUSE_MOVED = 0x06,
  EVENT_CODE_MOUSE_WHEEL = 0x07,
  EVENT_CODE_RESIZED = 0x08,

  EVENT_CODE_DEBUG0 = 0x10,
  EVENT_CODE_DEBUG1 = 0x11,
  EVENT_CODE_DEBUG2 = 0x12,
  EVENT_CODE_DEBUG3 = 0x13,
  EVENT_CODE_DEBUG4 = 0x14,

  MAX_EVENT_CODE = 0xff,
};

#endif // EVENT_HEADER_INCLUDED
