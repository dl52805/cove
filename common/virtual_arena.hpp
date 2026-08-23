#pragma once

#include <assert.h>
#include <stdio.h>

#include "def.hpp"

#include "os.hpp"

struct Virtual_Arena
{
  Virtual_Arena *prev;
  Virtual_Arena *current;
  u64 reserved_size;
  u64 commit_size;
  u64 committed;
  u64 base_offset;
  u64 offset;

  constexpr static u64 default_commit_size = 64 * 1024;
  constexpr static u64 default_reserve_size = gigabytes(1);

  static Virtual_Arena *create(u64 reserve = default_reserve_size,
                               u64 commit = default_commit_size)
  {
    u64 page = os_page_size();
    u64 reserve_align = align_to(reserve, page);
    u64 commit_align = align_to(commit, page);

    void *mem = os_reserve(reserve_align);
    os_commit(mem, commit_align);

    Virtual_Arena *arena = (Virtual_Arena *) mem;
    arena->reserved_size = reserve_align;
    arena->commit_size = commit_align;
    arena->committed = commit_align;
    arena->offset = sizeof(Virtual_Arena);
    arena->base_offset = 0;
    arena->prev = nullptr;
    arena->current = arena;

    return arena;
  }

  void *push(u64 size, u64 align = sizeof(uintptr_t))
  {
    Virtual_Arena *arena = current;
    u64 aligned_offset = align_to(arena->offset, align);
    u64 new_offset = aligned_offset + size;
    new_offset = align_to(new_offset, align);

    if (arena->reserved_size < new_offset)
    {
      u64 reserve = arena->reserved_size;
      u64 commit = arena->commit_size;

      u64 new_size = align_to(size, align)
                     + align_to(sizeof(Virtual_Arena), align);
      if (new_size > arena->reserved_size)
      {
        reserve = align_to(new_size, gigabytes(1));
        commit = align_to(new_size, os_page_size());
      }
      Virtual_Arena *new_arena = create(reserve, commit);
      new_arena->base_offset = arena->base_offset + arena->reserved_size;
      new_arena->prev = arena;
      this->current = new_arena;
      arena = new_arena;

      aligned_offset = align_to(arena->offset, align);
      new_offset = aligned_offset + size;
      new_offset = align_to(new_offset, align);
    }

    if (arena->committed < new_offset)
    {
      u64 commit_offset = align_to(new_offset, arena->commit_size);
      commit_offset = min(commit_offset, arena->reserved_size);
      u64 commit = commit_offset - arena->committed;
      u8 *commit_ptr = (u8 *) arena + arena->committed;
      os_commit(commit_ptr, commit);
      arena->committed = commit_offset;
    }

    void *result = nullptr;
    if (arena->committed >= new_offset)
    {
      result = (u8 *) arena + aligned_offset;
      arena->offset = new_offset;
    }

    return result;
  }

  // NOTE(dl): `size` should use `sizeof` and `align` should use `alignof`
  void *push_arr(u64 size, u64 n, u64 align = sizeof(uintptr_t))
  {
    u64 total_size = align_to(size, align) * n;

    Virtual_Arena *arena = current;
    u64 aligned_offset = align_to(arena->offset, align);
    u64 new_offset = aligned_offset + total_size;

    if (arena->reserved_size < new_offset)
    {
      u64 reserve = arena->reserved_size;
      u64 commit = arena->commit_size;

      u64 new_size = total_size + align_to(sizeof(Virtual_Arena), align);
      if (new_size > arena->reserved_size)
      {
        reserve = align_to(new_size, gigabytes(1));
        commit = align_to(new_size, os_page_size());
      }
      Virtual_Arena *new_arena = create(reserve, commit);
      new_arena->base_offset = arena->base_offset + arena->reserved_size;
      new_arena->prev = arena;
      this->current = new_arena;
      arena = new_arena;

      aligned_offset = align_to(arena->offset, align);
      new_offset = aligned_offset + total_size;
      new_offset = align_to(new_offset, align);
    }

    if (arena->committed < new_offset)
    {
      u64 commit_offset = align_to(new_offset, arena->commit_size);
      commit_offset = min(commit_offset, arena->reserved_size);
      u64 commit = commit_offset - arena->committed;
      u8 *commit_ptr = (u8 *) arena + arena->committed;
      os_commit(commit_ptr, commit);
      arena->committed = commit_offset;
    }

    void *result = nullptr;
    if (arena->committed >= new_offset)
    {
      result = (u8 *) arena + aligned_offset;
      arena->offset = new_offset;
    }

    return result;
  }

  void pop_to(u64 position)
  {
    u64 offset_adjust = max(position, sizeof(Virtual_Arena));

    Virtual_Arena *curr = this->current;
    Virtual_Arena *prev = nullptr;
    while (true)
    {
      if (curr->base_offset <= offset_adjust) break;
      prev = curr->prev;
      os_release(curr, curr->reserved_size);
      curr = prev;
    }
    this->current = curr;
    u64 new_offset = offset_adjust - curr->base_offset;
    curr->offset = new_offset;
  }

  u64 position()
  {
    return current->base_offset + current->offset;
  }

  void clear()
  {
    pop_to(0);
  }

  void deinit()
  {
    Virtual_Arena *curr = this->current;
    Virtual_Arena *prev = nullptr;
    while (true)
    {
      prev = curr->prev;
      os_release(curr, curr->reserved_size);
      if (!prev) break;
      curr = prev;
    }
  }
};

