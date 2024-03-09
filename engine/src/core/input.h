#ifndef INPUT_HEADER_INCLUDED
#define INPUT_HEADER_INCLUDED

#include "../defines.h"

enum ns_btn { NSB_LEFT, NSB_RIGHT, NSB_MIDDLE, NSB_MAX_BUTTONS };

#define DEFINE_KEY(name, code) NSK_##name = code

enum ns_key {
  DEFINE_KEY(BACKSPACE, 0x08),
  DEFINE_KEY(RETURN, 0x0d),
  DEFINE_KEY(TAB, 0x09),
  DEFINE_KEY(SHIFT, 0x10),
  DEFINE_KEY(CONTROL, 0x11),

  DEFINE_KEY(PAUSE, 0x13),
  DEFINE_KEY(CAPITAL, 0x14),

  DEFINE_KEY(ESCAPE, 0x1B),

  DEFINE_KEY(CONVERT, 0x1C),
  DEFINE_KEY(NONCONVERT, 0x1D),
  DEFINE_KEY(ACCEPT, 0x1E),
  DEFINE_KEY(MODECHANGE, 0x1F),

  DEFINE_KEY(SPACE, 0x20),
  DEFINE_KEY(PRIOR, 0x21),
  DEFINE_KEY(NEXT, 0x22),
  DEFINE_KEY(END, 0x23),
  DEFINE_KEY(HOME, 0x24),
  DEFINE_KEY(LEFT, 0x25),
  DEFINE_KEY(UP, 0x26),
  DEFINE_KEY(RIGHT, 0x27),
  DEFINE_KEY(DOWN, 0x28),
  DEFINE_KEY(SELECT, 0x29),
  DEFINE_KEY(PRINT, 0x2A),
  DEFINE_KEY(EXECUTE, 0x2B),
  DEFINE_KEY(SNAPSHOT, 0x2C),
  DEFINE_KEY(INSERT, 0x2D),
  DEFINE_KEY(DELETE, 0x2E),
  DEFINE_KEY(HELP, 0x2F),

  DEFINE_KEY(A, 0x41),
  DEFINE_KEY(B, 0x42),
  DEFINE_KEY(C, 0x43),
  DEFINE_KEY(D, 0x44),
  DEFINE_KEY(E, 0x45),
  DEFINE_KEY(F, 0x46),
  DEFINE_KEY(G, 0x47),
  DEFINE_KEY(H, 0x48),
  DEFINE_KEY(I, 0x49),
  DEFINE_KEY(J, 0x4A),
  DEFINE_KEY(K, 0x4B),
  DEFINE_KEY(L, 0x4C),
  DEFINE_KEY(M, 0x4D),
  DEFINE_KEY(N, 0x4E),
  DEFINE_KEY(O, 0x4F),
  DEFINE_KEY(P, 0x50),
  DEFINE_KEY(Q, 0x51),
  DEFINE_KEY(R, 0x52),
  DEFINE_KEY(S, 0x53),
  DEFINE_KEY(T, 0x54),
  DEFINE_KEY(U, 0x55),
  DEFINE_KEY(V, 0x56),
  DEFINE_KEY(W, 0x57),
  DEFINE_KEY(X, 0x58),
  DEFINE_KEY(Y, 0x59),
  DEFINE_KEY(Z, 0x5A),

  DEFINE_KEY(LWIN, 0x5B),
  DEFINE_KEY(RWIN, 0x5C),
  DEFINE_KEY(APPS, 0x5D),

  DEFINE_KEY(SLEEP, 0x5F),

  DEFINE_KEY(NUMPAD0, 0x60),
  DEFINE_KEY(NUMPAD1, 0x61),
  DEFINE_KEY(NUMPAD2, 0x62),
  DEFINE_KEY(NUMPAD3, 0x63),
  DEFINE_KEY(NUMPAD4, 0x64),
  DEFINE_KEY(NUMPAD5, 0x65),
  DEFINE_KEY(NUMPAD6, 0x66),
  DEFINE_KEY(NUMPAD7, 0x67),
  DEFINE_KEY(NUMPAD8, 0x68),
  DEFINE_KEY(NUMPAD9, 0x69),
  DEFINE_KEY(MULTIPLY, 0x6A),
  DEFINE_KEY(ADD, 0x6B),
  DEFINE_KEY(SEPARATOR, 0x6C),
  DEFINE_KEY(SUBTRACT, 0x6D),
  DEFINE_KEY(DECIMAL, 0x6E),
  DEFINE_KEY(DIVIDE, 0x6F),
  DEFINE_KEY(F1, 0x70),
  DEFINE_KEY(F2, 0x71),
  DEFINE_KEY(F3, 0x72),
  DEFINE_KEY(F4, 0x73),
  DEFINE_KEY(F5, 0x74),
  DEFINE_KEY(F6, 0x75),
  DEFINE_KEY(F7, 0x76),
  DEFINE_KEY(F8, 0x77),
  DEFINE_KEY(F9, 0x78),
  DEFINE_KEY(F10, 0x79),
  DEFINE_KEY(F11, 0x7A),
  DEFINE_KEY(F12, 0x7B),
  DEFINE_KEY(F13, 0x7C),
  DEFINE_KEY(F14, 0x7D),
  DEFINE_KEY(F15, 0x7E),
  DEFINE_KEY(F16, 0x7F),
  DEFINE_KEY(F17, 0x80),
  DEFINE_KEY(F18, 0x81),
  DEFINE_KEY(F19, 0x82),
  DEFINE_KEY(F20, 0x83),
  DEFINE_KEY(F21, 0x84),
  DEFINE_KEY(F22, 0x85),
  DEFINE_KEY(F23, 0x86),
  DEFINE_KEY(F24, 0x87),

  DEFINE_KEY(NUMLOCK, 0x90),
  DEFINE_KEY(SCROLL, 0x91),

  DEFINE_KEY(NUMPAD_EQUAL, 0x92),

  DEFINE_KEY(LSHIFT, 0xA0),
  DEFINE_KEY(RSHIFT, 0xA1),
  DEFINE_KEY(LCONTROL, 0xA2),
  DEFINE_KEY(RCONTROL, 0xA3),
  DEFINE_KEY(LALT, 0xA4),
  DEFINE_KEY(RALT, 0xA5),

  DEFINE_KEY(SEMICOLON, 0xBA),
  DEFINE_KEY(PLUS, 0xBB),
  DEFINE_KEY(COMMA, 0xBC),
  DEFINE_KEY(MINUS, 0xBD),
  DEFINE_KEY(PERIOD, 0xBE),
  DEFINE_KEY(SLASH, 0xBF),
  DEFINE_KEY(GRAVE, 0xC0),

  NSK_MAX_KEYS
};

#undef DEFINE_KEY

namespace ns {

class InputManager {
public:
  static void initialize(usize *memory_requirement, ptr state);
  static void cleanup(ptr state);
  static void update(f64 delta_time);
  static void process_key(ns_key key, bool pressed);
  static void process_button(ns_btn button, bool pressed);
  static void process_mouse_move(i16 x, i16 y);
  static void process_mouse_wheel(i8 z_delta);

  inline bool _k(usize);
  inline bool _pk(usize);
  inline bool _m(usize);
  inline bool _pm(usize);
  inline void _mp(i32 *, i32 *);
  inline void _pmp(i32 *, i32 *);
  inline void _mm(i32 *, i32 *);

private:
  bool keyboard[256];
  bool prev_keyboard[256];
  bool mouse[NSB_MAX_BUTTONS];
  bool prev_mouse[NSB_MAX_BUTTONS];
  i32 mouse_x;
  i32 mouse_y;
  i32 prev_mouse_x;
  i32 prev_mouse_y;
};

namespace keyboard {

NS_API bool pressed(ns_key key);
NS_API bool released(ns_key key);
NS_API bool down(ns_key key);
NS_API bool up(ns_key key);
NS_API bool was_down(ns_key key);
NS_API bool was_up(ns_key key);

} // namespace keyboard

namespace mouse {

NS_API bool pressed(ns_btn button);
NS_API bool released(ns_btn button);
NS_API bool down(ns_btn button);
NS_API bool up(ns_btn button);
NS_API bool was_down(ns_btn button);
NS_API bool was_up(ns_btn button);

NS_API void position(i32 *x, i32 *y);
NS_API void prev_position(i32 *x, i32 *y);
NS_API void motion(i32 *x, i32 *y);

} // namespace mouse

} // namespace ns

#endif // INPUT_HEADER_INCLUDED
