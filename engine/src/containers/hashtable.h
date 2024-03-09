#ifndef HASHTABLE_HEADER_INCLUDED
#define HASHTABLE_HEADER_INCLUDED

#include "../defines.h"

namespace ns {

struct chashtable {
  NS_API chashtable(usize element_size, u32 element_count, ptr memory,
                    bool is_pointer_type = false);
  NS_API ~chashtable();

  NS_API bool fill(ptr value);

  NS_API bool set(cstr name, ptr value);

  NS_API bool set_ptr(cstr name, ptr value);

  NS_API bool get(cstr name, ptr out_value) const;

  NS_API bool get_ptr(cstr name, ptr *out_value) const;

  template <typename T> NS_API bool get_ptr(cstr name, T **out_value) const {
    return get_ptr(name, reinterpret_cast<ptr *>(out_value));
  }

  // internal
  usize element_size;
  u32 element_count;
  bool is_pointer_type;
  ptr memory;

#if (_DEBUG)
  usize count;
#endif
};

} // namespace ns

#endif // HASHTABLE_HEADER_INCLUDED
