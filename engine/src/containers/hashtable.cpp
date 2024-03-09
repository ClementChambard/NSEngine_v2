#include "./hashtable.h"

#include "../core/logger.h"
#include "../core/ns_memory.h"

static u64 hash_name(cstr name, u32 element_count) {
  static const u64 multiplier = 97;

  robytes us;
  u64 hash = 0;

  for (us = reinterpret_cast<robytes>(name); *us; us++) {
    hash = hash * multiplier + *us;
  }

  return hash % element_count;
}

namespace ns {

chashtable::chashtable(usize element_size, u32 element_count, ptr memory,
                       bool is_pointer_type)
    : element_size(element_size), element_count(element_count),
      is_pointer_type(is_pointer_type), memory(memory) {
  if (!memory) {
    NS_ERROR("chashtable creation failed! Pointer to memory is required.");
    return; // TODO(ClementChambard): handle error more nicely
  }
  if (!element_count || !element_size) {
    NS_ERROR("chashtable creation failed! element_count and element_size must "
             "be a positive non-zero value.");
    return; // TODO(ClementChambard): handle error more nicely
  }

  mem_zero(memory, element_size * element_count);
}

chashtable::~chashtable() {
  if (memory) {
    mem_zero(memory, element_size * element_count);
  }
  memory = nullptr;
  element_size = 0;
  element_count = 0;
}

bool chashtable::fill(ptr value) {
  if (!memory || !value) {
    NS_ERROR("chashtable::fill requires valid memory and value to exist.");
    return false;
  }
  if (is_pointer_type) {
    NS_ERROR(
        "chashtable::fill shouldn't be used with pointer_type hashtables.");
    return false;
  }
  for (u32 i = 0; i < element_count; i++) {
    bytes memory_bytes = reinterpret_cast<bytes>(memory);
    mem_copy(memory_bytes + (element_size * i), value, element_size);
  }
  return true;
}

bool chashtable::set(cstr name, ptr value) {
  if (!memory || !name || !value) {
    NS_ERROR("chashtable::set requires valid memory, name and value");
    return false;
  }
  if (is_pointer_type) {
    NS_ERROR("chashtable::set shouldn't be used with pointer_type hashtables.");
    return false;
  }

  u64 hash = hash_name(name, element_count);
  bytes memory_bytes = reinterpret_cast<bytes>(memory);
  mem_copy(memory_bytes + (element_size * hash), value, element_size);
  return true;
}

bool chashtable::set_ptr(cstr name, ptr value) {
  if (!memory || !name) {
    NS_ERROR("chashtable::set_ptr requires valid memory and name");
    return false;
  }
  if (!is_pointer_type) {
    NS_ERROR("chashtable::set_ptr shouldn't be used with non-pointer_type "
             "hashtables.");
    return false;
  }

  u64 hash = hash_name(name, element_count);
  reinterpret_cast<ptr *>(memory)[hash] = value;
  return true;
}

bool chashtable::get(cstr name, ptr out_value) const {
  if (!memory || !name || !out_value) {
    NS_ERROR("chashtable::get requires valid memory, name and out_value");
    return false;
  }
  if (is_pointer_type) {
    NS_ERROR("chashtable::get shouldn't be used with pointer_type hashtables.");
    return false;
  }
  u64 hash = hash_name(name, element_count);
  bytes memory_bytes = reinterpret_cast<bytes>(memory);
  mem_copy(out_value, memory_bytes + (element_size * hash), element_size);
  return true;
}

bool chashtable::get_ptr(cstr name, ptr *out_value) const {
  if (!memory || !name || !out_value) {
    NS_ERROR("chashtable::get_ptr requires valid memory, name and out_value");
    return false;
  }
  if (!is_pointer_type) {
    NS_ERROR("chashtable::get_ptr shouldn't be used with non-pointer_type "
             "hashtables.");
    return false;
  }
  u64 hash = hash_name(name, element_count);
  *out_value = reinterpret_cast<ptr *>(memory)[hash];
  return *out_value != nullptr;
}

} // namespace ns
