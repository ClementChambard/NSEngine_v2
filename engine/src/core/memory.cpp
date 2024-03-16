#include "./memory.h"

#include "../platform/platform.h"
#include "./logger.h"
#include <cstdio>
#include <cstring>

namespace ns {

struct memory_stats {
  u64 total_allocated;
  u64 tagged_allocations[static_cast<usize>(MemTag::MAX_TAGS)];
};

NS_STATIC_ASSERT(static_cast<usize>(MemTag::MAX_TAGS) == 18,
                 "update memory tag strings");
static cstr memory_tag_strings[static_cast<usize>(MemTag::MAX_TAGS)] = {
    "UNKNOWN    ", "LINEAR_ALLC", "ARRAY      ", "VECTOR     ", "DICT       ",
    "RING_QUEUE ", "BST        ", "STRING     ", "APPLICATION", "JOB        ",
    "TEXTURE    ", "MAT_INST   ", "RENDERER   ", "GAME       ", "TRANSFORM  ",
    "ENTITY     ", "ENTITY_NODE", "SCENE      ",
};

struct memory_system_state {
  struct memory_stats stats;
  u64 alloc_count;
};

static memory_system_state *state_ptr;

void memory_system_initialize(usize *memory_requirement, ptr state) {
  *memory_requirement = sizeof(memory_system_state);
  if (state == nullptr) {
    return;
  }
  state_ptr = reinterpret_cast<memory_system_state *>(state);
  state_ptr->alloc_count = 0;
  platform::zero_memory(&state_ptr->stats, sizeof(memory_stats));
}

void memory_system_shutdown(ptr /*state*/) { state_ptr = nullptr; }

ptr alloc(usize size, MemTag tag) {
  if (tag == MemTag::UNKNOWN) {
    NS_WARN("ns::alloc called using mem_tag::UNKNOWN. Re-class this "
            "allocation.");
  }

  if (state_ptr) {
    state_ptr->stats.total_allocated += size;
    state_ptr->stats.tagged_allocations[static_cast<usize>(tag)] += size;
    state_ptr->alloc_count++;
  }

  // TODO(ClementChambard): memory alignment
  ptr block = platform::allocate_memory(size, false);
  platform::zero_memory(block, size);

  return block;
}

NS_API ptr realloc(ptr block, usize prev_size, usize new_size, MemTag tag) {
  if (tag == MemTag::UNKNOWN) {
    NS_WARN("ns::realloc called using mem_tag::UNKNOWN. Re-class this "
            "reallocation.");
  }

  if (state_ptr) {
    isize s_diff = new_size - prev_size;
    state_ptr->stats.total_allocated += s_diff;
    state_ptr->stats.tagged_allocations[static_cast<usize>(tag)] += s_diff;
  }

  return platform::reallocate_memory(block, new_size, false);
}

void free(ptr block, usize size, MemTag tag) {
  if (tag == MemTag::UNKNOWN) {
    NS_WARN("ns::free called using mem_tag::UNKNOWN. Re-class this "
            "free.");
  }

  if (state_ptr) {
    state_ptr->stats.total_allocated -= size;
    state_ptr->stats.tagged_allocations[static_cast<usize>(tag)] -= size;
  }

  // TODO(ClementChambard): memory alignment
  platform::free_memory(block, false);
}

ptr mem_zero(ptr block, usize size) {
  return platform::zero_memory(block, size);
}

ptr mem_copy(ptr dest, roptr source, usize size) {
  return platform::copy_memory(dest, source, size);
}

ptr mem_set(ptr dest, i32 value, usize size) {
  return platform::set_memory(dest, value, size);
}

pstr get_memory_usage_str() {
  if (!state_ptr)
    return nullptr;
  const usize gib = 1024 * 1024 * 1024;
  const usize mib = 1024 * 1024;
  const usize kib = 1024;

  char buffer[8000] = "System memory use (tagged):\n";
  usize offset = strlen(buffer);
  for (u32 i = 0; i < static_cast<u32>(MemTag::MAX_TAGS); ++i) {
    char unit[4] = "XiB";
    f32 amount = 1.0f;
    if (state_ptr->stats.tagged_allocations[i] >= gib) {
      unit[0] = 'G';
      amount = state_ptr->stats.tagged_allocations[i] / static_cast<f32>(gib);
    } else if (state_ptr->stats.tagged_allocations[i] >= mib) {
      unit[0] = 'M';
      amount = state_ptr->stats.tagged_allocations[i] / static_cast<f32>(mib);
    } else if (state_ptr->stats.tagged_allocations[i] >= kib) {
      unit[0] = 'K';
      amount = state_ptr->stats.tagged_allocations[i] / static_cast<f32>(kib);
    } else {
      unit[0] = 'B';
      unit[1] = '\0';
      amount = static_cast<f32>(state_ptr->stats.tagged_allocations[i]);
    }

    offset += snprintf(buffer + offset, sizeof(buffer) - offset,
                       "  %s: %.2f%s\n", memory_tag_strings[i], amount, unit);
  }

  pstr out_string = strdup(buffer);
  return out_string;
}

u64 get_memory_alloc_count() {
  if (state_ptr)
    return state_ptr->alloc_count;
  return 0;
}

} // namespace ns
