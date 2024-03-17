#include "./freelist.h"

#include "../core/logger.h"
#include "../core/memory.h"

namespace ns {

struct freelist_node {
  u32 offset;
  u32 size;
  freelist_node *next;
};

struct internal_state {
  u32 total_size;
  u32 max_entries;
  freelist_node *head;
  freelist_node *nodes;
};

freelist_node *get_node(freelist list);
void return_node(freelist list, freelist_node *node);

void freelist_create(u32 total_size, u64 *memory_requirement, void *memory,
                     freelist *out_list) {
  u32 max_entries = total_size / sizeof(void *);
  *memory_requirement =
      sizeof(internal_state) + max_entries * sizeof(freelist_node);
  if (memory == nullptr) {
    return;
  }

  usize mem_min = (sizeof(internal_state) + sizeof(freelist_node)) * 8;

  if (total_size < mem_min) {
    NS_WARN("Total size of freelist is too small. Re-class this freelist.");
  }

  out_list->memory = memory;

  mem_zero(memory, *memory_requirement);

  internal_state *state = reinterpret_cast<internal_state *>(memory);
  state->nodes = reinterpret_cast<freelist_node *>(state + 1);
  state->max_entries = max_entries;
  state->total_size = total_size;

  state->head = &state->nodes[0];
  state->head->offset = 0;
  state->head->size = total_size;
  state->head->next = nullptr;

  for (u32 i = 1; i < max_entries; i++) {
    state->nodes[i].offset = INVALID_ID;
    state->nodes[i].size = INVALID_ID;
  }
}

void freelist_destroy(freelist *list) {
  if (list && list->memory) {
    internal_state *state = reinterpret_cast<internal_state *>(list->memory);
    mem_zero(list->memory, sizeof(internal_state) +
                               sizeof(freelist_node) * state->max_entries);
    list->memory = nullptr;
  }
}

bool freelist_allocate_block(freelist list, usize size, u64 *out_offset) {
  if (!out_offset || !list.memory) {
    return false;
  }
  internal_state *state = reinterpret_cast<internal_state *>(list.memory);
  freelist_node *node = state->head;
  freelist_node *prev = nullptr;
  while (node) {
    if (node->size == size) {
      *out_offset = node->offset;
      freelist_node *node_to_return = nullptr;
      if (prev) {
        prev->next = node->next;
        node_to_return = node;
      } else {
        node_to_return = state->head;
        state->head = node->next;
      }
      return_node(list, node_to_return);
      return true;
    } else if (node->size > size) {
      *out_offset = node->offset;
      node->size -= size;
      node->offset += size;
      return true;
    }
    prev = node;
    node = node->next;
  }
  usize free_space = freelist_free_space(list);
  NS_WARN("freelist_find_block, no block with enough space found (requested: "
          "%uB, available: %lluB).",
          size, free_space);
  return false;
}

NS_API bool freelist_try_reallocate_block(freelist list, usize /*old_size*/,
                                          usize new_size, u64 /*offset*/,
                                          u64 *new_offset) {
  // TODO

  // in the meantime, just allocate a new block
  freelist_allocate_block(list, new_size, new_offset);
  return false;
}

bool freelist_free_block(freelist list, usize size, u64 offset) {
  if (!list.memory || !size) {
    return false;
  }
  internal_state *state = reinterpret_cast<internal_state *>(list.memory);
  freelist_node *node = state->head;
  freelist_node *prev = nullptr;
  if (!node) {
    freelist_node *new_node = get_node(list);
    new_node->offset = offset;
    new_node->size = size;
    new_node->next = nullptr;
    state->head = new_node;
    return true;
  }
  while (node) {
    if (node->offset == offset) {
      node->size += size;
      if (node->next && node->next->offset == node->offset + node->size) {
        node->size += node->next->size;
        freelist_node *next = node->next;
        node->next = next->next;
        return_node(list, next);
      }
      return true;
    } else if (node->offset > offset) {
      freelist_node *new_node = get_node(list);
      new_node->offset = offset;
      new_node->size = size;

      if (prev) {
        prev->next = new_node;
        new_node->next = node;
      } else {
        new_node->next = node;
        state->head = new_node;
      }

      if (new_node->next &&
          new_node->offset + new_node->size == new_node->next->offset) {
        new_node->size += new_node->next->size;
        freelist_node *next = new_node->next;
        new_node->next = next->next;
        return_node(list, next);
      }

      if (prev && prev->offset + prev->size == new_node->offset) {
        prev->size += new_node->size;
        freelist_node *next = new_node;
        prev->next = next->next;
        return_node(list, next);
      }

      return true;
    }

    prev = node;
    node = node->next;
  }

  NS_WARN("freelist_free_block, no block with offset %u found. Corruption "
          "possible ?",
          offset);
  return true;
}

void freelist_clear(freelist list) {
  if (!list.memory) {
    return;
  }
  internal_state *state = reinterpret_cast<internal_state *>(list.memory);
  for (u32 i = 0; i < state->max_entries; i++) {
    state->nodes[i].offset = INVALID_ID;
    state->nodes[i].size = INVALID_ID;
  }
  state->head->offset = 0;
  state->head->size = state->total_size;
  state->head->next = nullptr;
}

usize freelist_free_space(freelist list) {
  if (!list.memory) {
    return 0;
  }
  internal_state *state = reinterpret_cast<internal_state *>(list.memory);
  usize free_space = 0;
  freelist_node *node = state->head;
  while (node) {
    free_space += node->size;
    node = node->next;
  }
  return free_space;
}

freelist_node *get_node(freelist list) {
  internal_state *state = reinterpret_cast<internal_state *>(list.memory);
  for (u32 i = 0; i < state->max_entries; i++) {
    if (state->nodes[i].offset == INVALID_ID) {
      return &state->nodes[i];
    }
  }

  return nullptr;
}

void return_node(freelist, freelist_node *node) {
  node->offset = INVALID_ID;
  node->size = INVALID_ID;
  node->next = nullptr;
}

} // namespace ns
