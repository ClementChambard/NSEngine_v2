#include "./platform.h"

#if NS_PLATFORM_WINDOWS

#include "../containers/vector.h"
#include "../core/event.h"
#include "../core/input.h"
#include "../core/logger.h"

#include <stdlib.h>
#include <windows.h>
#include <windowsx.h>

#include "../renderer/vulkan/vulkan_types.inl"
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_win32.h>

namespace ns::platform {

struct platform_state {
  HINSTANCE h_instance;
  HWND hwnd;
  VkSurfaceKHR surface;
};

static platform_state *state_ptr;

static f64 clock_frequency;
static LARGE_INTEGER start_time;

LRESULT CALLBACK win32_process_message(HWND hwnd, u32 msg, WPARAM w_param,
                                       LPARAM, l_param);

void clock_setup() {
  LARGE_INTEGER frequency;
  QueryPerformanceFrequency(&frequency);
  clock_frequency = 1.0 / static_cast<f64>(frequency.QuadPart);
  QueryPerformanceCounter(&start_time);
}

bool startup(u64 memory_requirement, ptr state, cstr application_name, i32 x,
             i32 y, i32 width, i32 height) {
  *memory_requirement = sizeof(platform_state);
  if (state == nullptr) {
    return true;
  }
  state_ptr = reinterpret_cast<platform_state *>(state);

  state_ptr->h_instance = GetModuleHandleA(0);

  HICON icon = LoadIcon(state_ptr->h_instance, IDI_APPLICATION);
  WNDCLASSA wc;
  memset(&wc, 0, sizeof(wc));
  wc.style = CS_DBLCLKS;
  wc.lpfnWndProc = win32_process_message;
  wc.cbClsExtra = 0;
  wc.cbWndExtra = 0;
  wc.hInstance = state_ptr->h_instance;
  wc.hIcon = icon;
  wc.hCursor = LoadCursor(NULL, IDC_ARROW);
  wc.hbrBackground = NULL;
  wc.lpszClassName = "ns_window_class";

  if (!RegisterClassA(&wc)) {
    MessageBoxA(0, "Window registration failed", "Error",
                MB_ICONEXCLAMATION | MB_OK);
    return false;
  }

  u32 client_x = x;
  u32 client_y = y;
  u32 client_width = width;
  u32 client_height = height;

  u32 window_x = client_x;
  u32 window_y = client_y;
  u32 window_width = client_width;
  u32 window_height = client_height;

  u32 window_style = WS_OVERLAPPED | WS_SYSMENU | WS_CAPTION;
  u32 window_ex_style = WS_EX_APPWINDOW;

  window_style |= WS_MAXIMIZEBOX;
  window_style |= WS_MINIMIZEBOX;
  window_style |= WS_THICKFRAME;

  RECT border_rect = {0, 0, 0, 0};
  AdjustWindowRectEx(&border_rect, window_style, 0, window_ex_style);

  window_x += border_rect.left;
  window_y += border_rect.top;

  window_width += border_rect.right - border_rect.left;
  window_height += border_rect.bottom - border_rect.top;

  HWND handle =
      CreateWindowExA(window_ex_style, "ns_window_class", application_name,
                      window_style, window_x, window_y, window_width,
                      window_height, 0, 0, state_ptr->h_instance, 0);

  if (handle == 0) {
    MessageBoxA(NULL, "Window creation failed!", "Error!",
                MB_ICONEXCLAMATION | MB_OK);

    NS_FATAL("Window creation failed!");
    return false;
  } else {
    state_ptr->hwnd = handle;
  }

  bool should_activate = 1; // TODO(ClementChambard): if the window should not
                            // accept input, this should be false.
  i32 show_window_command_flags = should_activate ? SW_SHOW : SW_SHOWNOACTIVATE;
  ShowWindow(state_ptr->hwnd, show_window_command_flags);

  clock_setup();

  return true;
}

void shutdown(ptr plat_state) {
  if (state_ptr && state_ptr->hwnd) {
    DestroyWindow(state_ptr->hwnd);
    state_ptr->hwnd = 0;
  }
}

bool pump_messages() {
  if (!state_ptr)
    return true;
  MSG message;
  while (PeekMessageA(&message, NULL, 0, 0, PM_REMOVE)) {
    TranslateMessage(&message);
    DispatchMessageA(&message);
  }
  return true;
}

ptr allocate_memory(usize size, bool /* aligned */) {
  return std::malloc(size);
}

ptr reallocate_memory(ptr block, usize new_size, bool /* aligned */) {
  return std::realloc(block, new_size);
}

void free_memory(ptr block, bool /* aligned */) { std::free(block); }

ptr zero_memory(ptr block, usize size) { return memset(block, 0, size); }

ptr copy_memory(ptr dest, roptr source, usize size) {
  return memcpy(dest, source, size);
}

ptr set_memory(ptr dest, i32 value, usize size) {
  return memset(dest, value, size);
}

void console_write(cstr message, u8 color) {
  HANDLE console_handle = GetStdHandle(STD_OUTPUT_HANDLE);
  static u8 levels[6] = {64, 4, 6, 2, 1, 8};
  SetConsoleTextAttribute(console_handle, levels[color]);

  OutputDebugStringA(message);
  DWORD length = reinterpret_cast<DWORD>(strlen(message));
  LPDWORD number_written = 0;
  WriteConsoleA(GetStdHandle(STD_OUTPUT_HANDLE), message, length,
                number_written, 0);
}

void console_write_error(cstr message, u8 color) {
  HANDLE console_handle = GetStdHandle(STD_ERROR_HANDLE);
  static u8 levels[6] = {64, 4, 6, 2, 1, 8};
  SetConsoleTextAttribute(console_handle, levels[color]);

  OutputDebugStringA(message);
  DWORD length = reinterpret_cast<DWORD>(strlen(message));
  LPDWORD number_written = 0;
  WriteConsoleA(GetStdHandle(STD_ERROR_HANDLE), message, length, number_written,
                0);
}

f64 get_absolute_time() {
  if (!clock_frequency)
    clock_setup();
  LARGE_INTEGER now_time;
  QueryPerformanceCounter(&now_time);
  return static_cast<f64>(now_time.QuadPart) * clock_frequency;
}

void sleep(u64 ms) { Sleep(ms); }

void get_required_extension_names(vector<cstr> *names_darray) {
  names_darray->push_back("VK_KHR_win32_surface");
}

bool create_vulkan_surface(vulkan::Context *context) {
  if (!state_ptr)
    return false;
  VkWin32SurfaceCreateInfoKHR create_info;
  create_info.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
  create_info.hinstance = state_ptr->h_instance;
  create_info.hwnd = state_ptr->hwnd;

  VkResult result = vkCreateWin32SurfaceKHR(
      context->instance, &create_info, context->allocator, &state_ptr->surface);
  if (result != VK_SUCCESS) {
    NS_FATAL("Vulkan surface creation failed.");
    return false;
  }

  context->surface = state_ptr->surface return true;
}

LRESULT CALLBACK win32_process_message(HWND hwnd, u32 msg, WPARAM w_param,
                                       LPARAM, l_param) {
  switch (msg) {
  case WM_ERASEBKGND:
    return 1;
  case WM_CLOSE:
    event_context data = {};
    event_fire(EVENT_CODE_APPLICATION_QUIT, 0, data);
    return 1;
  case WM_DESTROY:
    PostQuitMessage(0);
    return 0;
  case WM_SIZE: {
    RECT r;
    GetClientRect(hwnd, &r);
    u32 width = r.right - r.left;
    u32 height = r.bottom - r.top;
    event_context context;
    context.data.U16[0] = width;
    context.data.U16[1] = height;
    event_fire(EVENT_CODE_RESIZED, 0, context);
  } break;
  case WM_KEYDOWN:
  case WM_SYSKEYDOWN:
  case WM_KEYUP:
  case WM_SYSKEYUP: {
    ns_key key = static_cast<u16>(w_param);
    bool is_extended = (HIWORD(l_param) & KF_EXTENDED) == KF_EXTENDED;
    if (w_param == VK_MENU) {
      key = is_extended ? KEY_RALT : KEY_LALT;
    } else if (w_param == VK_SHIFT) {
      u32 left_shift = MapVirtualKey(VK_LSHIFT, MAPVK_VK_TO_VSC);
      u32 scancode = ((l_param & (0xFF << 16)) >> 16);
      key = scancode == left_shift ? KEY_LSHIFT : KEY_RSHIFT;
    } else if (w_param == VK_CONTROL) {
      key = is_extended ? KEY_RCONTROL : KEY_LCONTROL;
    }
    InputManager::process_key(key, msg == WM_KEYDOWN || msg == WM_SYSKEYDOWN);
  } break;
  case WM_MOUSEMOVE: {
    InputManager::process_mouse_move(GET_X_LPARAM(l_param),
                                     GET_Y_LPARAM(l_param));
  } break;
  case WM_MOUSEWHEEL: {
    i32 z_delta = GET_WHEEL_DELTA_WPARAM(w_param);
    if (z_delta != 0) {
      InputManager::process_mouse_wheel((z_delta < 0) ? -1 : 1);
    }
  } break;
  case WM_LBUTTONDOWN:
  case WM_MBUTTONDOWN:
  case WM_RBUTTONDOWN:
  case WM_LBUTTONUP:
  case WM_MBUTTONUP:
  case WM_RBUTTONUP: {
    bool pressed =
        msg == WM_LBUTTONDOWN || msg == WM_RBUTTONDOWN || msg == WM_MBUTTONDOWN;
    buttons mouse_button = NSB_MAX_BUTTONS;
    switch (msg) {
    case WM_LBUTTONDOWN:
    case WM_LBUTTONUP:
      mouse_button = NSB_LEFT;
      break;
    case WM_MBUTTONDOWN:
    case WM_MBUTTONUP:
      mouse_button = NSB_MIDDLE;
      break;
    case WM_RBUTTONDOWN:
    case WM_RBUTTONUP:
      mouse_button = NSB_RIGHT;
      break;
    }

    if (mouse_button != NSB_MAX_BUTTONS) {
      InputManager::process_button(mouse_button, pressed);
    }
  } break;
  }

  return DefWindowProcA(hwnd, msg, w_param, l_param);
}

} // namespace ns::platform

#endif // NS_PLATFORM_WINDOWS
